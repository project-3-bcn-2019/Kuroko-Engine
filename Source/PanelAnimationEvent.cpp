#include "PanelAnimationEvent.h"
#include "Application.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "ComponentAnimationEvent.h"
#include "ComponentAnimation.h"
#include "ModuleResourcesManager.h"
#include "ResourceAnimation.h"
#include "Component.h"
#include "FileSystem.h"
#include "GameObject.h"

PanelAnimationEvent::PanelAnimationEvent(const char* name) : Panel(name)
{
	active = false;

	zoom = 25;
	numFrames = 100;
	recSize = 1000;
	speed = 0.5f;
	progress = 0.0f;
	winSize = 1000.0f;
}

bool PanelAnimationEvent::fillInfo()
{
	bool ret = false;

	if (App->scene->selected_obj.size() == 1)
		selected_obj = App->scene->selected_obj.begin()._Ptr->_Myval;

	if (selected_obj != nullptr)
	{
		auto get = (ComponentAnimationEvent*)selected_obj->getComponent(Component_type::ANIMATION_EVENT);
		if (get != c_AnimEvt)
		{
			c_AnimEvt = get;
			c_AnimEvt->getParent()->getComponents(par_components);
			selected_component = (ComponentAnimation*)par_components.front();
			
				/*uint get_uid = c_AnimEvt->getAnimationResource();
				animation = (ResourceAnimation*)App->resources->getResource(c_AnimEvt->getAnimationResource());

				numFrames = 0;
				if (animation != nullptr)
					numFrames = animation->ticks;
				// Create a new Animation Resource that will control the animation of this node
				*/
				
			}
		}

	if (c_AnimEvt != nullptr)
	{
		if (selected_component == nullptr)
			selected_component = c_AnimEvt->getParent()->getComponent(Component_type::TRANSFORM);

		ret = true;
	}

	return ret;
}

PanelAnimationEvent::~PanelAnimationEvent()
{
}

void PanelAnimationEvent::Draw()
{
	ImGui::Begin(name.c_str(), &active, ImGuiWindowFlags_HorizontalScrollbar);

	if (fillInfo())
	{
		if (ImGui::BeginCombo("Components", selected_component->TypeToString().c_str()))
		{
			//c_AnimEvt->getParent()->getComponents(par_components);

			for (std::list<Component*>::iterator it = par_components.begin(); it != par_components.end(); ++it)
				if (it._Ptr->_Myval->getType() != Component_type::ANIMATION && it._Ptr->_Myval->getType() != Component_type::ANIMATION_EVENT && ImGui::Selectable(it._Ptr->_Myval->TypeToString().c_str(), &it._Ptr->_Myval->AnimSel)) {
					selected_component->AnimSel = false;
					selected_component = it._Ptr->_Myval;
				}
			ImGui::EndCombo();

			//par_components.clear();
		}

		if (selected_component->getType() != Component_type::ANIMATION && selected_component->getType() != Component_type::ANIMATION_EVENT)
		{
			if (ImGui::Button("Add Key")) new_keyframe_win = true;
			ImGui::SameLine();
			if (ImGui::Button("Del Key")) del_keyframe_win = true;
			ImGui::SameLine();
			if (ImGui::Button("Delete All Keys"))
			{
				auto get = c_AnimEvt->curr->AnimEvts.find(selected_component->getUUID());
				if (get != c_AnimEvt->curr->AnimEvts.end())
					get->second.clear();
			}

			if (new_keyframe_win)
				AddKeyframe();

			if (del_keyframe_win)
				DeleteKeyframe();

			ImGui::BeginGroup();
			ImGui::SetCursorPosY(85);
			ImGui::Text("Component Keys");
			ImGui::EndGroup();

			if (selected_component != nullptr)
			{
				//Animation TimeLine
				ImGui::BeginChild("TimeLine", ImVec2(winSize, 180), true, ImGuiWindowFlags_HorizontalScrollbar);
				ImVec2 p = ImGui::GetCursorScreenPos();
				ImVec2 redbar = ImGui::GetCursorScreenPos();
				//ImGui::InvisibleButton("scrollbar", { numFrames*zoom ,ImGui::GetWindowSize().y });
				ImGui::SetCursorScreenPos(p);

				int check_size = c_AnimEvt->curr->own_ticks;
				// When linked to a compAnimation
				for (int i = 0; i < check_size; i++)
				{
					ImGui::BeginGroup();

					ImGui::GetWindowDrawList()->AddLine({ p.x,p.y }, ImVec2(p.x, p.y + ImGui::GetWindowSize().y), IM_COL32(100, 100, 100, 255), 1.0f);

					char frame[8];
					sprintf(frame, "%01d", i);
					ImVec2 aux = { p.x + 3,p.y };
					ImGui::GetWindowDrawList()->AddText(aux, ImColor(1.0f, 1.0f, 1.0f, 1.0f), frame);

					if (c_AnimEvt != nullptr && selected_component != nullptr)
					{
						if (c_AnimEvt->AnimEvts.size() > 0)
						{
							auto get = c_AnimEvt->curr->AnimEvts.find(selected_component->getUUID());
							if (get != c_AnimEvt->curr->AnimEvts.end() && get->second.size() > 0)
							{
								if (get->second.find(i) != get->second.end())
									ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(p.x + 1, p.y + 35), 6.0f, ImColor(1.0f, 1.0f, 1.0f, 0.5f));
							}
						}
					}

					p = { p.x + zoom,p.y };

					ImGui::EndGroup();

					//ImGui::SameLine();

				}

				ImGui::EndChild();
			}

		}

	}

	ImGui::End();
	
}

void PanelAnimationEvent::AddKeyframe()
{
	ImGui::Begin("New Keyframe");
	{
		if (ImGui::BeginCombo("Event Types", selected_component->EvTypetoString(ev_t).c_str()))
		{
			for (int i = -1; i < selected_component->getEvAmount(); ++i)
				if (ImGui::Selectable(selected_component->EvTypetoString(i).c_str())) {
					ev_t = i;
				}
			ImGui::EndCombo();
		}
		int anim_ticks = c_AnimEvt->curr->own_ticks;
		// Depend on a linked to compAnimation later on
		ImGui::DragInt("Frame", &new_key_frame, 1, 0, anim_ticks);
		// Add a method that allows to input value depending on the event and component i guess, every component will have it

		if (ImGui::Button("Create"))
		{
			if (new_key_frame < anim_ticks && new_key_frame >= 0)
			{
				new_keyframe_win = false;

				KeyframeVals mpush;
				mpush.insert(std::pair<int, void*>(ev_t, nullptr));
				KeyMap m2push;
				m2push.insert(std::pair<double, KeyframeVals>(new_key_frame, mpush));
				int prev_size = c_AnimEvt->AnimEvts.size();
				auto ret_ins = c_AnimEvt->curr->AnimEvts.insert(std::pair<uint, KeyMap>(selected_component->getUUID(), m2push));

				if (!ret_ins.second)
				{
					auto ret_comp = ret_ins.first;
					auto ret_ins_comp = ret_comp->second.insert(std::pair<double, KeyframeVals>(new_key_frame, mpush));
					if (!ret_ins_comp.second)
					{
						auto ret_keymap = ret_ins_comp.first;
						auto ret_ins_keymap = ret_keymap->second.insert(std::pair<int, void*>(ev_t, nullptr));
						if (!ret_ins_keymap.second)
							ret_ins_keymap.first->second = nullptr; //here the value should be updated
					}
				}
			}
			else
			{
				// send error message you fucked up the ticks
			}
			ResetNewKeyValues();
		}

		if (ImGui::Button("Cancel")) ResetNewKeyValues();
	}
	ImGui::End();
}

void PanelAnimationEvent::DeleteKeyframe()
{

	ImGui::Begin("Del Keyframe");
	auto get = c_AnimEvt->curr->AnimEvts.find(selected_component->getUUID());
	if (get == c_AnimEvt->curr->AnimEvts.end() || get->second.size() <= 0)
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "NO KEYFRAMES SET!");
	else
	{
		if (ImGui::BeginCombo("Keyframes Available", std::to_string(get->second.begin()->first).c_str()))
		{
			for (auto it = get->second.begin(); it != get->second.end(); ++it)
				if (ImGui::Selectable(std::to_string(it->first).c_str()))
					key_to_del = it->first;

			ImGui::EndCombo();
		}
		
	}

	if(key_to_del != -1)
		if (ImGui::Button("Delete"))
		{
			get->second.erase(get->second.find(key_to_del));
			del_keyframe_win = false;
		}

	if (ImGui::Button("Cancel"))
	{
		key_to_del = -1;
		del_keyframe_win = false;
	}

	ImGui::End();
}

void PanelAnimationEvent::CopySpecs()
{
	fillInfo();
	ComponentAnimation* c_cast = (ComponentAnimation*)c_AnimEvt->getParent()->getComponent(Component_type::ANIMATION);
	
	if (c_cast != nullptr)
	{
		c_AnimEvt->curr->loop = c_cast->loop;
		c_AnimEvt->curr->speed = c_cast->speed;

		if (c_cast->getAnimationResource() != 0)
		{
			ResourceAnimation* get = (ResourceAnimation*)App->resources->getResource(c_cast->getAnimationResource());
			if (get != nullptr)
			{
				c_AnimEvt->curr->linked_animation = get->uuid;

				if (get->ticks < c_AnimEvt->curr->own_ticks)
				{
					for (auto it = c_AnimEvt->curr->AnimEvts.begin(); it != c_AnimEvt->curr->AnimEvts.end(); ++it)
					{
						for (int i = c_AnimEvt->curr->own_ticks; i > get->ticks; i--)
						{
							auto get_key = it->second.find(i);
							if (get_key != it->second.end())
								it->second.erase(get_key);
						}
					}
				}

				c_AnimEvt->curr->own_ticks = get->ticks;

				c_AnimEvt->curr->ticksXsecond = get->ticksXsecond;

			}
		}
	}
}
#include "PanelAnimation.h"
#include "Application.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "ComponentAnimation.h"
#include "ModuleResourcesManager.h"
#include "ResourceAnimation.h"
#include "Component.h"
#include "FileSystem.h"

PanelAnimation::PanelAnimation(const char* name) : Panel(name)
{
	active = false;

	zoom = 25;
	numFrames = 100;
	recSize = 1000;
	speed = 0.5f;
	progress = 0.0f;
	winSize = 1000.0f;
}

bool PanelAnimation::fillInfo()
{
	bool ret = false;

	if (App->scene->selected_obj != nullptr)
	{
		compAnimation = (ComponentAnimation*)App->scene->selected_obj->getComponent(Component_type::ANIMATION);

		if (compAnimation != nullptr)
		{
			if(selected_component == nullptr)
				selected_component = compAnimation;
			uint get_uid = compAnimation->getAnimationResource();
			animation = (ResourceAnimation*)App->resources->getResource(compAnimation->getAnimationResource());
			
			if (animation != nullptr)
			{
				// Create a new Animation Resource that will control the animation of this node
				//App->resources->newResource(ResourceAnimation();
				ret = true;
				numFrames = animation->ticks;
			}
			else
			{
				// Create empty resource/file to edit
				//App->fs.
			}
		}
	}

	return ret;
}

PanelAnimation::~PanelAnimation()
{
}

void PanelAnimation::Draw()
{

	ImGui::Begin(name.c_str(), &active, ImGuiWindowFlags_HorizontalScrollbar);
	
	

	if (fillInfo())
	{
		if (ImGui::BeginCombo("Components", selected_component->TypeToString().c_str()))
		{
			compAnimation->getParent()->getComponents(par_components);

			for (std::list<Component*>::iterator it = par_components.begin(); it != par_components.end(); ++it)
				if (ImGui::Selectable(it._Ptr->_Myval->TypeToString().c_str(), &it._Ptr->_Myval->AnimSel)) {
					selected_component->AnimSel = false;
					selected_component = it._Ptr->_Myval;
				}
				ImGui::EndCombo();
		}

		//Animation bar Progress
		/*ImGui::SetCursorPosX(85);
		//ImGui::ProgressBar((compAnimation->animTime / animation->getDuration()), { winSize,0 });
		ImGui::SameLine();
		if (ImGui::Button("Play"))
		{
			compAnimation->TestPlay = true;
			compAnimation->TestPause = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Pause"))
		{
			compAnimation->TestPause = !compAnimation->TestPause;
			mouseMovement.x = progress;
			buttonPos = progress;
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop") && compAnimation->TestPlay)
		{
			compAnimation->TestPlay = false;
			compAnimation->animTime = 0.0f;
			compAnimation->TestPause = false;
		}*/

		// Space for another line of tools or whatever needed

		if (ImGui::Button("Add Keyframe")) new_keyframe_win = true;
		if(new_keyframe_win)
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
				ImGui::DragInt("Frame",&new_key_frame,1,0,animation->ticks);
				// Add a method that allows to input value depending on the event and component i guess, every component will have it

				if(ImGui::Button("Create"))
				{
					if (new_key_frame < animation->ticks && new_key_frame >= 0)
					{
						new_keyframe_win = false;

						/*auto get_comp = compAnimation->ComponentAnimations.find(selected_component->getUUID());
						if (get_comp != compAnimation->ComponentAnimations.end())
						{
							auto get_key_map = get_comp->second.find(new_key_frame);
							if (get_key_map != get_comp->second.end())
							{
								get_key_map->second.insert(std::pair<int, void*>(ev_t, nullptr));
							}
							else
							{
								std::map<int, void*> mpush;
								mpush.insert(std::pair<int, void*>(ev_t, nullptr));
								get_comp->second.insert(std::pair<double, std::map<int, void*>>(new_key_frame, mpush));
							}
						}
						else
						{*/
							KeyframeVals mpush;
							mpush.insert(std::pair<int, void*>(ev_t, nullptr));
							KeyMap m2push;
							m2push.insert(std::pair<double, KeyframeVals>(new_key_frame, mpush));
							int prev_size = compAnimation->ComponentAnimations.size();
							auto ret_ins = compAnimation->ComponentAnimations.insert(std::pair<uint, KeyMap>(selected_component->getUUID(), m2push));
							if(!ret_ins.second)
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
						/*}*/
					}
					else
					{
						// send error message you fucked up the ticks
					}
				}

				if (ImGui::Button("Cancel")) new_keyframe_win = false;
			}
			ImGui::End();
		}

		//Animation typeos of Keys

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
			ImGui::InvisibleButton("scrollbar", { numFrames*zoom ,ImGui::GetWindowSize().y});
			ImGui::SetCursorScreenPos(p);
			
			for (int i = 0; i < numFrames; i++)
			{
				ImGui::BeginGroup();

				ImGui::GetWindowDrawList()->AddLine({ p.x,p.y }, ImVec2(p.x, p.y +ImGui::GetWindowSize().y), IM_COL32(100, 100, 100, 255), 1.0f);

				char frame[8];
				sprintf(frame, "%01d", i);
				ImVec2 aux = { p.x + 3,p.y };
				ImGui::GetWindowDrawList()->AddText(aux, ImColor(1.0f, 1.0f, 1.0f, 1.0f), frame);

				if (compAnimation != nullptr && selected_component != nullptr)
				{
					if (compAnimation->ComponentAnimations.size() > 0)
					{
						auto get = compAnimation->ComponentAnimations.find(selected_component->getUUID());
						if (get != compAnimation->ComponentAnimations.end() && get->second.size() > 0)
						{
							if(get->second.find(i) != get->second.end())
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
		/*ImGui::BeginGroup();

		ImGui::SetCursorPosY(85);

		ImGui::Text("Position");

		ImGui::SetCursorPosY(125);

		ImGui::Text("Scale");

		ImGui::SetCursorPosY(165);

		ImGui::Text("Rotation");

		ImGui::EndGroup();

		ImGui::SameLine();

		//Animation TimeLine
		ImGui::BeginChild("TimeLine", ImVec2(winSize, 180), true, ImGuiWindowFlags_HorizontalScrollbar);
		ImVec2 p = ImGui::GetCursorScreenPos();
		ImVec2 redbar = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton("scrollbar", { numFrames*zoom ,140 });
		ImGui::SetCursorScreenPos(p);

		for (int i = 0; i < numFrames; i++)
		{
			ImGui::BeginGroup();

			ImGui::GetWindowDrawList()->AddLine({ p.x,p.y }, ImVec2(p.x, p.y + 135), IM_COL32(100, 100, 100, 255), 1.0f);

			char frame[8];
			sprintf(frame, "%01d", i);
			ImVec2 aux = { p.x + 3,p.y };
			ImGui::GetWindowDrawList()->AddText(aux, ImColor(1.0f, 1.0f, 1.0f, 1.0f), frame);

			if (animation != nullptr && selectedBone != nullptr)
			{
				if (selectedBone->PosKeysTimes[i] == i)
				{
					ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(p.x + 1, p.y + 35), 6.0f, ImColor(0.0f, 0.0f, 1.0f, 0.5f));
				}
				if (selectedBone->ScaleKeysTimes[i] == i)
				{
					ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(p.x + 1, p.y + 75), 6.0f, ImColor(0.0f, 1.0f, 0.0f, 0.5f));
				}
				if (selectedBone->RotKeysTimes[i] == i)
				{
					ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(p.x + 1, p.y + 115), 6.0f, ImColor(1.0f, 0.0f, 0.0f, 0.5f));
				}
			}

			p = { p.x + zoom,p.y };

			ImGui::EndGroup();

			ImGui::SameLine();

		}

		//RedLine 
		if (!App->inGameMode() && !compAnimation->TestPlay && !compAnimation->TestPause)
		{
			ImGui::GetWindowDrawList()->AddLine({ redbar.x,redbar.y - 10 }, ImVec2(redbar.x, redbar.y + 165), IM_COL32(255, 0, 0, 100), 1.0f);
			progress = 0.0f;
			ImGui::SetScrollX(0);
		}
		else if (!compAnimation->TestPause)
		{
			float auxprgbar = progress;

			ImGui::GetWindowDrawList()->AddLine({ redbar.x + progress,redbar.y - 10 }, ImVec2(redbar.x + progress, redbar.y + 165), IM_COL32(255, 0, 0, 255), 1.0f);

			if (!App->isGamePaused())
			{
				progress = (compAnimation->animTime*animation->ticksXsecond)*zoom;
				scrolled = false;
			}

			float framesInWindow = winSize / zoom;

			if (progress != 0 && progress > winSize && !scrolled)
			{
				float scroolPos = ImGui::GetScrollX();
				ImGui::SetScrollX(scroolPos + winSize);
				scrolled = true;
			}

			if (auxprgbar > progress)
			{
				progress = 0.0f;
				ImGui::SetScrollX(0);
			}
		}

		if (compAnimation->TestPause)
		{
			ImGui::SetCursorPos({ buttonPos,ImGui::GetCursorPosY() + 140 });
			ImGui::PushID("scrollButton");
			ImGui::Button("", { 20, 15 });
			ImGui::PopID();

			if (ImGui::IsItemClicked(0) && dragging == false)
			{
				dragging = true;
				offset = ImGui::GetMousePos().x - ImGui::GetWindowPos().x - buttonPos;
			}

			if (dragging && ImGui::IsMouseDown(0))
			{
				buttonPos = ImGui::GetMousePos().x - ImGui::GetWindowPos().x - offset;
				if (buttonPos < 0)
					buttonPos = 0;
				if (buttonPos > numFrames*zoom - 20)
					buttonPos = numFrames * zoom - 20;

				progress = buttonPos;
				compAnimation->animTime = progress / (animation->ticksXsecond *zoom);

			}
			else
			{
				dragging = false;
			}

			ImGui::GetWindowDrawList()->AddLine({ redbar.x + progress,redbar.y - 10 }, ImVec2(redbar.x + progress, redbar.y + 165), IM_COL32(255, 0, 0, 255), 1.0f);
		}

		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginGroup();

		ImGui::BeginChild("All Animations", ImVec2(250, 140), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

		if (animation != nullptr)
		{
			for (int i = 0; i < animation->numBones; i++)
			{
				if (ImGui::Button(animation->boneTransformations[i].NodeName.c_str()))
				{
					selectedBone = &animation->boneTransformations[i];
				}
			}
		}

		ImGui::EndChild();

		//ImGui::SameLine();

		ImGui::BeginChild("Selected Bone", ImVec2(250, 30), true);

		if (selectedBone != nullptr)
		{
			ImGui::Text(selectedBone->NodeName.c_str());
		}

		ImGui::EndChild();
		ImGui::EndGroup();

		ImGui::NewLine();*/
	}

	ImGui::End();
}
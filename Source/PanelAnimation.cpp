#include "PanelAnimation.h"
#include "Application.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "ComponentAnimation.h"
#include "ModuleResourcesManager.h"
#include "ResourceAnimation.h"
#include "Component.h"
#include "FileSystem.h"
#include "GameObject.h"
#include "ModuleTimeManager.h"
#include "ComponentBone.h"

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
	
	if (App->scene->selected_obj.size() == 1)
		selected_obj = App->scene->selected_obj.begin()._Ptr->_Myval;

	if (selected_obj != nullptr)
	{
		compAnimation = (ComponentAnimation*)selected_obj->getComponent(Component_type::ANIMATION);

		if (compAnimation != nullptr)
		{
			uint get_uid = compAnimation->getAnimationResource();
			animation = (ResourceAnimation*)App->resources->getResource(compAnimation->getAnimationResource());
			
			if (animation != nullptr)
			{
				numFrames = animation->ticks;
				// Create a new Animation Resource that will control the animation of this node
				if (selectedBone == nullptr)
				{
					selectedBone = animation->boneTransformations;
					GameObject* get_go = compAnimation->getParent()->getChildByUUID(compAnimation->GetCBone(0));
					if (get_go != nullptr)
						compBone = (ComponentBone*)get_go->getComponent(Component_type::BONE);
					else
						compBone = nullptr;
				}
				ret = true;
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
		winSize = animation->ticks * 20;
		if (winSize > 400) winSize = 400;
		if (winSize < 100) winSize = 400;

		//Animation bar Selection
		ImGui::PushItemWidth(winSize);
		ImGui::SliderInt("##CurrFrame", &frames, 0, animation->ticks - 1);
		compAnimation->SetAnimTime(frames / animation->ticksXsecond);

		ImGui::ProgressBar((compAnimation->GetAnimTime() / (animation->getDuration() - 1 / animation->ticksXsecond)), ImVec2(winSize, 0));
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
			compAnimation->SetAnimTime(0.0f);
			compAnimation->TestPause = false;
		}

		// Space for another line of tools or whatever needed



		//Animation typeos of Keys
		ImGui::BeginChild("##FrameTypes", ImVec2(95, 130), true, ImGuiWindowFlags_HorizontalScrollbar);
		{
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "KeyTypes");

			ImGui::BeginGroup();
			ImVec2 p = ImGui::GetCursorScreenPos();
			ImGui::GetWindowDrawList()->AddLine(p, ImVec2(p.x + 75, p.y), IM_COL32(100, 100, 100, 255), 2.f);

			ImGui::SetCursorPosY(30);

			ImGui::Text("Position");

			ImGui::SetCursorPosY(50);

			ImGui::Text("Scale");

			ImGui::SetCursorPosY(70);

			ImGui::Text("Rotation");

			ImGui::SetCursorPosY(90);
			if (compBone != nullptr) ImGui::Text("Components");

			ImGui::EndGroup();

		}	ImGui::EndChild();
		ImGui::SameLine();

		//Animation TimeLine

		ImGui::BeginChild("TimeLine", ImVec2(winSize, 130), true, ImGuiWindowFlags_HorizontalScrollbar);
		ImVec2 p = ImGui::GetCursorScreenPos();
		ImVec2 redbar = ImGui::GetCursorScreenPos();

		ImGui::GetWindowDrawList()->AddLine(ImVec2(p.x, p.y + 20), ImVec2(p.x + numFrames * 25, p.y + 20), IM_COL32(100, 100, 100, 255), 2.f);


		ImGui::InvisibleButton("scrollbar", { numFrames*zoom ,110 });

		ImGui::SetCursorScreenPos(p);


		for (int i = 0; i < numFrames; i++)
		{
			ImGui::BeginGroup();

			ImGui::GetWindowDrawList()->AddLine({ p.x,p.y }, ImVec2(p.x, p.y + 100), IM_COL32(100, 100, 100, 255), 1.0f);
			ImGui::GetWindowDrawList()->AddLine(ImVec2(p.x, p.y + 40), ImVec2(p.x + numFrames * 25, p.y + 40), IM_COL32(100, 100, 100, 150), 1.f);
			ImGui::GetWindowDrawList()->AddLine(ImVec2(p.x, p.y + 60), ImVec2(p.x + numFrames * 25, p.y + 60), IM_COL32(100, 100, 100, 150), 1.f);
			ImGui::GetWindowDrawList()->AddLine(ImVec2(p.x, p.y + 80), ImVec2(p.x + numFrames * 25, p.y + 80), IM_COL32(100, 100, 100, 150), 1.f);
			char frame[8];
			sprintf(frame, "%01d", i);
			ImVec2 aux = { p.x + 3,p.y };
			ImGui::GetWindowDrawList()->AddText(aux, ImColor(1.0f, 1.0f, 1.0f, 1.0f), frame);

			if (animation != nullptr && selectedBone != nullptr)
			{
				if (selectedBone->PosKeysTimes[i] == i)
					ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x, p.y + 21), ImVec2(p.x + 25, p.y + 40), ImColor(0.0f, 0.0f, 1.0f, 0.5f));

				if (selectedBone->ScaleKeysTimes[i] == i)
					ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x, p.y + 41), ImVec2(p.x + 25, p.y + 60), ImColor(1.0f, 0.0f, 0.0f, 0.5f));

				if (selectedBone->RotKeysTimes[i] == i)
					ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x, p.y + 61), ImVec2(p.x + 25, p.y + 80), ImColor(0.0f, 1.0f, 0.0f, 0.5f));

				// Component Keys
				if (compBone != nullptr)
				{
					ImGui::SetCursorPos(ImVec2((i*25.f), 90));
					if (ImGui::InvisibleButton("TestInvisButton", ImVec2(15, 15)))
						bool caca = true;
				}

			}

			p = { p.x + zoom,p.y };

			ImGui::EndGroup();

			ImGui::SameLine();

		}

		//RedLine
		if (!(App->time->getGameState() == PLAYING) && !compAnimation->TestPlay && !compAnimation->TestPause)
		{
			ImGui::GetWindowDrawList()->AddLine({ redbar.x + frames * 25,redbar.y - 10 }, ImVec2(redbar.x + frames * 25, redbar.y + 100), IM_COL32(255, 255, 100, 255), 1.0f);
			ImGui::GetWindowDrawList()->AddRectFilled({ redbar.x + frames * 25,redbar.y - 10 }, ImVec2(redbar.x + frames * 25 + 25, redbar.y + 100), IM_COL32(255, 255, 100, 50));
			progress = 0.0f;
			//ImGui::SetScrollX(0);
		}

		else if (!compAnimation->TestPause)
		{
			float auxprgbar = progress;

			ImGui::GetWindowDrawList()->AddLine({ redbar.x + progress,redbar.y - 10 }, ImVec2(redbar.x + progress, redbar.y + 165), IM_COL32(255, 0, 0, 255), 1.0f);

			if (!(App->time->getGameState() == PAUSED))
			{
				progress = (compAnimation->GetAnimTime()*animation->ticksXsecond)*zoom;
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
	compAnimation->SetAnimTime(progress / (animation->ticksXsecond *zoom));

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


		if (animation != nullptr)
		{
			ImGui::BeginChild("All Animations", ImVec2(0, 95), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

			for (int i = 0; i < animation->numBones; i++)
			{
				if (ImGui::Button(animation->boneTransformations[i].NodeName.c_str()))
				{
					selectedBone = &animation->boneTransformations[i];
					GameObject* get_go = compAnimation->getParent()->getChildByUUID(compAnimation->GetCBone(i));
					if (get_go != nullptr)
						compBone = (ComponentBone*)get_go->getComponent(Component_type::BONE);
					else
						compBone = nullptr;
				}
			}
			ImGui::EndChild();

			ImGui::BeginChild("Selected Bone", ImVec2(0, 31), true, ImGuiWindowFlags_NoScrollbar);

			if (compBone != nullptr)
			{
				ImGui::Text("Linked to: %s", compBone->getParent()->getName().c_str());
			}

			ImGui::EndChild();
		}

		ImGui::EndGroup();

		if (compBone != nullptr)
		{
			token_false = false;
			ImGui::BeginGroup();
			{
				ImGui::BeginChild("Component Events", ImVec2(0, 0), true);
				{
					compBone->getParent()->getComponents(par_components);
					if (ImGui::Button("+")) TryPushKey();
					ImGui::SameLine();

					ImGui::PushItemWidth(250.f);
					if (sel_comp == nullptr)
					{
						if (ImGui::BeginCombo("##ComponentsToAnim", "SelectComponent"))
						{
							for (auto it_comps = par_components.begin(); it_comps != par_components.end(); ++it_comps)
								if (ImGui::Selectable(it_comps._Ptr->_Myval->TypeToString().c_str(), token_false))
									sel_comp = *it_comps;
							ImGui::EndCombo();
						}
					}
					else
					{
						if (ImGui::BeginCombo("##ComponentsToAnim", sel_comp->TypeToString().c_str()))
						{
							for (auto it_comps = par_components.begin(); it_comps != par_components.end(); ++it_comps)
								if (ImGui::Selectable(it_comps._Ptr->_Myval->TypeToString().c_str(), token_false))
									sel_comp = *it_comps;
							ImGui::EndCombo();
						}

						ImGui::SameLine();
						ImGui::PushItemWidth(250.f);
						if (ImGui::BeginCombo("##AnimEvtsSel", sel_comp->EvTypetoString(PushEvt.first).c_str()))
						{
							for (int i = 0; i < sel_comp->getEvAmount(); ++i)
								if (ImGui::Selectable(sel_comp->EvTypetoString(i).c_str(), token_false))
									PushEvt.first = i;

							ImGui::EndCombo();
						}
						// ImGui::SameLine();
						// Function to continue the values for the initialization of event
					}

										

				} ImGui::EndChild();
			} ImGui::EndGroup();
		}
		else
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "No associated GameObject, events unavailable!");
	}

	ImGui::End();
}

void PanelAnimation::TryPushKey()
{
	if (sel_comp != nullptr && PushEvt.first != -1)
	{
		auto GetAnimSet = compBone->AnimSets.find(animation->uuid);
		if(GetAnimSet != compBone->AnimSets.end())
		{
			auto getKeymap = GetAnimSet->second.AnimEvts.find(frames);
			if (getKeymap != GetAnimSet->second.AnimEvts.end())
			{
				auto getCompMap = getKeymap->second.find(sel_comp->getUUID());
				if (getCompMap != getKeymap->second.end())
				{
					getCompMap->second.insert(PushEvt);
				}
			}
		}
	}
	else
	{
		// Data not completed show warning
	}
}
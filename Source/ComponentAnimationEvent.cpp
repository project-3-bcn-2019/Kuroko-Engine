#include "ComponentAnimationEvent.h"

#include "Application.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ModuleTimeManager.h"
#include "ComponentAnimation.h"
#include "ModuleUI.h"
#include "PanelAnimationEvent.h"

ComponentAnimationEvent::ComponentAnimationEvent(JSON_Object* deff, GameObject* parent) : Component(parent, ANIMATION_EVENT)
{
	JSON_Array* AnimArr = json_value_get_array(json_object_get_value(deff, "AnimArr"));
	for (int i = 0; i < json_array_get_count(AnimArr); ++i)
	{
		AnimSet push;

		JSON_Object* AnimSet = json_value_get_object(json_array_get_value(AnimArr, i));

		push.loop = json_object_get_boolean(AnimSet, "loop");
		push.speed = json_object_get_number(AnimSet, "speed");

		// resource if needed

		push.own_ticks = json_object_get_number(AnimSet, "own_ticks");
		push.ticksXsecond = json_object_get_number(AnimSet, "ticksXsecond");

		push.name = json_object_get_string(AnimSet, "name");
		push.linked_animation = json_object_get_number(AnimSet, "linked_animation");

		JSON_Array* comp_arr = json_value_get_array(json_object_get_value(AnimSet, "comp_arr"));

		for (int i = 0; i < json_array_get_count(comp_arr); ++i)
		{
			JSON_Object* comp_obj = json_value_get_object(json_array_get_value(comp_arr, i));
			JSON_Array* keys_arr = json_value_get_array(json_object_get_value(comp_obj, "keys_array"));

			std::pair<uint, KeyMap> push_comp_anim;
			
			push_comp_anim.first = json_object_get_number(comp_obj, "obj_uuid");
			
			for (int j = 0; j < json_array_get_count(keys_arr); ++j)
			{
				JSON_Object* key_obj = json_value_get_object(json_array_get_value(keys_arr, j));
				JSON_Value* events_arr_val = json_object_get_value(key_obj, "event_array");
				JSON_Array* events_arr = json_value_get_array(events_arr_val);
				std::pair<double, std::map<int, void*>> push_key;

				push_key.first = json_object_get_number(key_obj, "keyframe");

				for (int k = 0; k < json_array_get_count(events_arr); ++k)
				{
					std::pair<int, void*> push_evt;
					JSON_Object* event_obj = json_value_get_object(json_array_get_value(events_arr, k));
					push_evt.first = json_object_get_number(event_obj, "event");
					// Get the value for the event, when available
					push_evt.second = nullptr;

					push_key.second.insert(push_evt);
				}
				
				push_comp_anim.second.insert(push_key);
			}
			
			push.AnimEvts.insert(push_comp_anim);
		}

		AnimEvts.push_back(push);
	}

	// assing the resource
}

ComponentAnimationEvent::~ComponentAnimationEvent()
{
	// Clear the values added in the event values
	// Everything should be handled by some function
	// in component functions i suppose, will do later
}

bool ComponentAnimationEvent::Update(float dt)
{
	std::list<Component*> components;
	parent->getComponents(components);
	if (curr == nullptr && AnimEvts.size() > 0)
		curr = &AnimEvts.front();

	if(true) CheckLinkAnim();

	if (App->time->getGameState() == PLAYING && !isPaused() && curr != nullptr && !((ComponentAnimation*)parent->getComponent(Component_type::ANIMATION))->isPaused())
	{
		animTime += dt * curr->speed;
		if (animTime * curr->ticksXsecond > curr->own_ticks && curr->loop)
			animTime -= (curr->own_ticks / (float)curr->ticksXsecond);	

	}	

	if (curr != nullptr && App->time->getGameState() == PLAYING)
	{
		int expected_keyframe = animTime * curr->ticksXsecond;
		if (expected_keyframe != last_frame)
		{
			for (auto it = curr->AnimEvts.begin(); it != curr->AnimEvts.end(); ++it)
			{
				for (auto it_components = components.begin(); it_components != components.end(); ++it_components)
				{
					if (it_components._Ptr->_Myval->getUUID() == it->first)
					{
						// Get the keyframe of animation currently and if so then

						auto find_key = it->second.find(expected_keyframe);
						if (find_key != it->second.end())
							it_components._Ptr->_Myval->ProcessAnimationEvents(find_key->second);

						last_frame = expected_keyframe;
						//break;
					}
				}
			}
		}
	}

	return true;
}

bool ComponentAnimationEvent::DrawInspector(int id)
{
	if (ImGui::CollapsingHeader("Animation Events"))
	{
		//ResourceAnimation* R_anim = (ResourceAnimation*)App->resources->getResource(anim->getAnimationResource());
		//ImGui::Text("Resource: %s", (R_anim != nullptr) ? R_anim->asset.c_str() : "None");

		static bool set_animation_menu = false;


		if (ImGui::Button("Create Animation"))
			App->gui->p_anim_evt->new_anim_set_win = true;
		if (App->gui->p_anim_evt->new_anim_set_win)
		{
			ImGui::Begin("Name", &App->gui->p_anim_evt->new_anim_set_win);

			ImGui::InputText("#SetName", App->gui->p_anim_evt->prov, 50);
			if (ImGui::Button("Create"))
			{
				AnimSet push;
				push.name = App->gui->p_anim_evt->prov;
				App->gui->p_anim_evt->new_anim_set_win = false;


				AnimEvts.push_back(push);
				if (curr != nullptr)
					curr->selected = false;
				curr = &AnimEvts.back();
				App->gui->p_anim_evt->curr = --AnimEvts.end();
				//p_anim_evt->prov = "\0";
			}

			ImGui::End();
		}

		if (curr == nullptr && AnimEvts.size() > 0)
		{
			if (ImGui::BeginCombo("Animation Sets", curr->name.c_str()))
			{

				for (auto it = AnimEvts.begin(); it != AnimEvts.end(); ++it)
					if (ImGui::Selectable(it->name.c_str(), &it->selected))
					{
						curr->selected = false;
						curr = &it._Ptr->_Myval;
						curr->selected = true;

						for (auto it = AnimEvts.begin(); it != AnimEvts.end(); ++it)
							if (&it._Ptr->_Myval == curr)
							{
								App->gui->p_anim_evt->curr = it;
								break;
							}
					}
				ImGui::EndCombo();
			}

			ImGui::SameLine();
			if (ImGui::Button("Delete"))
			{
				AnimEvts.erase(App->gui->p_anim_evt->curr);
				curr = nullptr;
				App->gui->p_anim_evt->curr = AnimEvts.end();
			}
		}

		if (curr != nullptr)
		{
			if (ImGui::BeginCombo("Animation Sets##12", curr->name.c_str()))
			{

				for (auto it = AnimEvts.begin(); it != AnimEvts.end(); ++it)
					if (ImGui::Selectable(it->name.c_str(), &it->selected))
					{
						curr->selected = false;
						curr = &it._Ptr->_Myval;
						curr->selected = true;

						for (auto it = AnimEvts.begin(); it != AnimEvts.end(); ++it)
							if (&it._Ptr->_Myval == curr)
							{
								App->gui->p_anim_evt->curr = it;
								break;
							}
					}
				ImGui::EndCombo();
			}

			if (ImGui::Button("Link Animation")) App->gui->p_anim_evt->copy_specs_win = true;

			App->gui->p_anim_evt->CopySpecs();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35);
				auto get = getParent()->getComponent(Component_type::ANIMATION);
				if (get != nullptr)
					ImGui::TextUnformatted(("Link the component animation to the\nskeletal animation, if available\n Currently linked to: " + get->TypeToString()).c_str());
				else
					ImGui::TextUnformatted(("No Animation to link to"));
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}

			static bool animation_active;
			animation_active = isActive();

			if (ImGui::Checkbox("Active##active animation evt", &animation_active))
				setActive(animation_active);

			ImGui::Checkbox("Loop", &curr->loop);


			ImGui::InputInt("Duration", &curr->own_ticks, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35);
				ImGui::TextUnformatted("In Frames");
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}
			ImGui::InputInt("FrameRate", &curr->ticksXsecond, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);

			ImGui::InputFloat("Speed", &curr->speed, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35);
				ImGui::TextUnformatted("Animation Speed Multiplier");
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}
			//if (R_anim != nullptr)
			{
				if (App->time->getGameState() != GameState::PLAYING)
				{
					ImGui::Text("Play");
					ImGui::SameLine();
					ImGui::Text("Pause");
				}
				else if (isPaused())
				{
					if (ImGui::Button("Play"))
						Play();
					ImGui::SameLine();
					ImGui::Text("Pause");
				}
				else
				{
					ImGui::Text("Play");
					ImGui::SameLine();
					if (ImGui::Button("Pause"))
						Pause();
				}

				ImGui::Text("Animation info:");
				if (curr->ticksXsecond != 0)
					ImGui::Text("Duration: %.1f ms", curr->own_ticks / curr->ticksXsecond * 1000.f);
				//ImGui::Text(" Animation Bones: %d", R_anim->numBones);
			}

			if (ImGui::Button("AnimEditor"))
				App->gui->p_anim_evt->toggleActive();
			if (App->gui->p_anim_evt->isActive())
				App->gui->p_anim_evt->Draw();
		}

		/*if (ImGui::Button("Remove Component##Remove animation"))
			ret = false;*/
	}
	return true;
}

void ComponentAnimationEvent::Save(JSON_Object* config)
{
	json_object_set_string(config, "type", "animation_event");

	JSON_Value* AnimList = json_value_init_array();
	JSON_Array* AnimArr = json_value_get_array(AnimList);

	for (auto it_sets = AnimEvts.begin(); it_sets != AnimEvts.end(); ++it_sets)
	{
		JSON_Value* AnimSetVal = json_value_init_object();
		JSON_Object* AnimSetObj = json_value_get_object(AnimSetVal);
		json_object_set_boolean(AnimSetObj, "loop", it_sets->loop);
		json_object_set_number(AnimSetObj, "speed", it_sets->speed);

		json_object_set_number(AnimSetObj, "own_ticks", it_sets->own_ticks);
		json_object_set_number(AnimSetObj, "ticksXsecond", it_sets->ticksXsecond);

		json_object_set_number(AnimSetObj, "linked_anim", it_sets->linked_animation);
		json_object_set_string(AnimSetObj, "name", it_sets->name.c_str());


		JSON_Value* comp_arr_val = json_value_init_array();
		JSON_Array* comp_arr = json_value_get_array(comp_arr_val);

		for (auto it = it_sets->AnimEvts.begin(); it !=  it_sets->AnimEvts.end(); ++it)
		{
			JSON_Value*  comp_val = json_value_init_object();
			JSON_Object* comp_obj = json_value_get_object(comp_val);
			json_object_set_number(comp_obj, "obj_uuid", it->first);

			JSON_Value* keys_arr_val = json_value_init_array();
			JSON_Array* keys_arr = json_value_get_array(keys_arr_val);

			for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
			{
				JSON_Value* key_val = json_value_init_object();
				JSON_Object* key_obj = json_value_get_object(key_val);
				json_object_set_number(key_obj, "keyframe", it2->first);

				JSON_Value* event_arr_val = json_value_init_array();
				JSON_Array* event_arr = json_value_get_array(event_arr_val);

				for (auto it3 = it2->second.begin(); it3 != it2->second.end(); ++it3)
				{
					JSON_Value* event_val = json_value_init_object();
					JSON_Object* event_obj = json_value_get_object(event_val);
					json_object_set_number(event_obj, "event", it3->first);
					// Now the value required should be save but currently not using it
					// When values are available for anim a switch for the value
					// adn depending on type of event and module should be saved

					json_array_append_value(event_arr, event_val);
				}
				json_object_set_value(key_obj, "event_array", event_arr_val);

				json_array_append_value(keys_arr, key_val);
			}
			json_object_set_value(comp_obj, "keys_array", keys_arr_val);

			json_array_append_value(comp_arr, comp_val);
		}

		json_object_set_value(AnimSetObj, "comp_arr", comp_arr_val);
		json_array_append_value(AnimArr, AnimSetVal);
	}
	json_object_set_value(config, "AnimArr", AnimList);
}

void ComponentAnimationEvent::CheckLinkAnim()
{
	ComponentAnimation* get = (ComponentAnimation*)parent->getComponent(Component_type::ANIMATION);
	if (get != nullptr && curr != nullptr)
	{
		if (curr->linked_animation != get->getAnimationResource())
		{
			for (auto it = AnimEvts.begin(); it != AnimEvts.end(); ++it)
			{
				if (it->linked_animation = get->getAnimationResource())
				{
					paused = false;
					curr = &it._Ptr->_Myval;
					break;
				}
				paused = true;
			}
		}
	}
}
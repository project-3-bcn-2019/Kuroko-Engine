#include "ComponentAnimationEvent.h"

#include "Application.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ModuleTimeManager.h"
#include "ComponentAnimation.h"

ComponentAnimationEvent::ComponentAnimationEvent(JSON_Object* deff, GameObject* parent) : Component(parent, ANIMATION_EVENT)
{
	JSON_Array* AnimArr = json_value_get_array(json_object_get_value(deff, "AnimArr"));
	for (int i = 0; i < json_array_get_count(AnimArr); ++i)
	{
		AnimSet push;

		push.loop = json_object_get_boolean(deff, "loop");
		push.speed = json_object_get_number(deff, "speed");

		// resource if needed

		push.own_ticks = json_object_get_number(deff, "own_ticks");
		push.ticksXsecond = json_object_get_number(deff, "ticksXsecond");

		push.name = json_object_get_string(deff, "name");
		push.linked_animation = json_object_get_number(deff, "linked_animation");

		JSON_Value* comp_arr_val = json_object_get_value(deff, "comp_arr");
		JSON_Array* comp_arr = json_value_get_array(comp_arr_val);

		for (int i = 0; i < json_array_get_count(comp_arr); ++i)
		{
			JSON_Object* comp_obj = json_value_get_object(json_array_get_value(comp_arr, i));
			JSON_Value* keys_arr_val = json_object_get_value(comp_obj, "keys_array");
			JSON_Array* keys_arr = json_value_get_array(keys_arr_val);

			std::pair<uint, KeyMap> push_comp_anim;
			for (int j = 0; j < json_array_get_count(keys_arr); ++j)
			{
				JSON_Object* key_obj = json_value_get_object(json_array_get_value(keys_arr, j));
				JSON_Value* events_arr_val = json_object_get_value(key_obj, "event_array");
				JSON_Array* events_arr = json_value_get_array(events_arr_val);
				std::pair<double, std::map<int, void*>> push_key;

				for (int k = 0; k < json_array_get_count(events_arr); ++k)
				{
					std::pair<int, void*> push_evt;
					JSON_Object* event_obj = json_value_get_object(json_array_get_value(events_arr, k));
					push_evt.first = json_object_get_number(event_obj, "event");
					// Get the value for the event, when available
					push_evt.second = nullptr;

					push_key.second.insert(push_evt);
				}
				push_key.first = json_object_get_number(key_obj, "keyframe");

				push_comp_anim.second.insert(push_key);
			}
			push_comp_anim.first = json_object_get_number(comp_obj, "obj_uuid");

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

	if (!isPaused() && curr != nullptr)
	{
		animTime += dt * curr->speed;
		if (animTime * curr->ticksXsecond > curr->own_ticks && curr->loop)
			animTime -= (curr->own_ticks / (float)curr->ticksXsecond);
	}	

	if (curr != nullptr)
	{
		CheckLinkAnim();
		for (auto it = curr->AnimEvts.begin(); it != curr->AnimEvts.end(); ++it)
		{
			for (auto it_components = components.begin(); it_components != components.end(); ++it_components)
			{
				if (it_components._Ptr->_Myval->getUUID() == it->first)
				{
					// Get the keyframe of animation currently and if so then
					int expected_keyframe = animTime * curr->ticksXsecond;
					/*
					if (animation_resource_uuid != 0)
					{
						Resource* get = App->resources->getResource(animation_resource_uuid);
						if (get != nullptr)
							expected_keyframe = animTime * ((ResourceAnimation*)get)->ticksXsecond;
					}*/
					auto find_key = it->second.find(expected_keyframe);
					if (find_key != it->second.end())
						it_components._Ptr->_Myval->ProcessAnimationEvents(find_key->second);
					break;
				}
			}
		}
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
		json_array_append_value(AnimArr, comp_arr_val);
	}
	json_object_set_value(config, "AnimArr", AnimList);
}

void ComponentAnimationEvent::CheckLinkAnim()
{
	ComponentAnimation* get = (ComponentAnimation*)parent->getComponent(Component_type::ANIMATION);
	if (get != nullptr)
	{
		if (curr->linked_animation != get->getAnimationResource())
		{
			for (auto it = AnimEvts.begin(); it != AnimEvts.end(); ++it)
			{
				if (it->linked_animation = get->getAnimationResource())
				{
					curr = &it._Ptr->_Myval;
					break;
				}
			}
		}
	}
}
#include "ComponentAnimationEvent.h"

#include "Application.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ModuleTimeManager.h"

ComponentAnimationEvent::ComponentAnimationEvent(JSON_Object* deff, GameObject* parent) : Component(parent, ANIMATION_EVENT)
{
	loop = json_object_get_boolean(deff, "loop");
	speed = json_object_get_number(deff, "speed");

	// resource if needed

	own_ticks = json_object_get_number(deff, "own_ticks");

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

		AnimEvts.insert(push_comp_anim);
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
	for (auto it = AnimEvts.begin(); it != AnimEvts.end(); ++it)
	{
		for (auto it_components = components.begin(); it_components != components.end(); ++it_components)
		{
			if (it_components._Ptr->_Myval->getUUID() == it->first)
			{
				// Get the keyframe of animation currently and if so then
				int expected_keyframe = 0;
				if (animation_resource_uuid != 0)
				{
					Resource* get = App->resources->getResource(animation_resource_uuid);
					if (get != nullptr)
						expected_keyframe = animTime * ((ResourceAnimation*)get)->ticksXsecond;
				}
				auto find_key = it->second.find(expected_keyframe);
				if (find_key != it->second.end())
					it_components._Ptr->_Myval->ProcessAnimationEvents(find_key->second);
				break;
			}
		}
	}
}

void ComponentAnimationEvent::Save(JSON_Object* config)
{
	json_object_set_number(config, "own_ticks", own_ticks);

	JSON_Value* comp_arr_val = json_value_init_array();
	JSON_Array* comp_arr = json_value_get_array(comp_arr_val);

	for (auto it = AnimEvts.begin(); it != AnimEvts.end(); ++it)
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
	json_object_set_value(config, "comp_arr", comp_arr_val);

}
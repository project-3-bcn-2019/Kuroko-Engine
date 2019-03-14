#include "ComponentBone.h"
#include "Application.h"
#include "ModuleResourcesManager.h"
#include "ResourceBone.h"
#include "ModuleRenderer3D.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "ImGui/imgui.h"

ComponentBone::ComponentBone(JSON_Object* deff, GameObject* parent) : Component(parent, BONE)
{
	const char* parent3dobject = json_object_get_string(deff, "Parent3dObject");
	if (App->is_game && !App->debug_game)
	{
		bone_resource_uuid = App->resources->getResourceUuid(json_object_get_string(deff, "bone_name"), R_BONE);
	}
	else if (parent3dobject) // Means that is being loaded from a scene
		bone_resource_uuid = App->resources->getBoneResourceUuid(parent3dobject, json_object_get_string(deff, "bone_name"));
	else // Means it is being loaded from a 3dObject binary
		bone_resource_uuid = json_object_get_number(deff, "bone_resource_uuid");

	App->resources->assignResource(bone_resource_uuid);

	JSON_Value* get_val = json_object_get_value(deff, "AnimArr");
	if (get_val != NULL)
	{
		JSON_Array* AnimArr = json_value_get_array(get_val);
		for (int i = 0; i < json_array_get_count(AnimArr); ++i)
		{
			AnimSetB push;
			JSON_Object* AnimSet = json_value_get_object(json_array_get_value(AnimArr, i));
			push.linked_animation = json_object_get_number(AnimSet, "linked_animiation");

			JSON_Value* key_val = json_object_get_value(AnimSet, "key_arr");
			if (key_val != NULL)
			{
				JSON_Array* key_arr = json_value_get_array(key_val);

				for (int j = 0; j < json_array_get_count(key_arr); ++j)
				{
					JSON_Object* key_obj = json_value_get_object(json_array_get_value(key_arr, j));
					JSON_Value* comp_val = json_object_get_value(key_obj, "comp_arr");
					if (comp_val != NULL)
					{
						JSON_Array* comp_arr = json_value_get_array(comp_val);

						std::pair<double, std::map<uint, std::map<int, void*>>> push_key;
						push_key.first = json_object_get_number(key_obj, "keyframe");
						for (int k = 0; k < json_array_get_count(comp_arr); ++k)
						{
							JSON_Object* comp_obj = json_value_get_object(json_array_get_value(comp_arr, k));
							JSON_Value* event_val = json_object_get_value(comp_obj, "event_arr");
							if (event_val != NULL)
							{
								JSON_Array* event_arr = json_value_get_array(event_val);

								std::pair<uint, std::map<int, void*>> push_comp;
								push_comp.first = json_object_get_number(comp_obj, "uuid");

								for (int l = 0; l < json_array_get_count(event_arr); ++l)
								{
									std::pair<int, void*> push_evt;
									JSON_Object* event_obj = json_value_get_object(json_array_get_value(event_arr, l));
									push_evt.first = json_object_get_number(event_obj, "event");
									// Get the values for the event when available
									push_evt.second = nullptr;
									push_comp.second.insert(push_evt);
								}
								push_key.second.insert(push_comp);
							}
							
						}
						push.AnimEvts.insert(push_key);
					}
					
				}
				AnimSets.insert(std::pair<uint, AnimSetB>(push.linked_animation, push));
			}
			
		}

	}
}

ComponentBone::~ComponentBone()
{
}

void ComponentBone::Save(JSON_Object * config)
{
	json_object_set_string(config, "type", "bone");

	if (bone_resource_uuid != 0)
	{		
		ResourceBone* res_bone = (ResourceBone*)App->resources->getResource(bone_resource_uuid);
		if (res_bone)
		{
			json_object_set_string(config, "bone_name", res_bone->asset.c_str());
			json_object_set_string(config, "Parent3dObject", res_bone->Parent3dObject.c_str());
		}
		else
		{
			json_object_set_string(config, "bone_name", "missing reference");
			json_object_set_string(config, "Parent3dObject", "missing reference");
		}

		if (AnimSets.size() > 0)
		{
			JSON_Value* AnimList = json_value_init_array();
			JSON_Array* AnimSetArr = json_value_get_array(AnimList);

			for (auto it_sets = AnimSets.begin(); it_sets != AnimSets.end(); ++it_sets)
			{
				JSON_Value* AnimSetVal = json_value_init_object();
				JSON_Object* AnimSetObj = json_value_get_object(AnimSetVal);
				json_object_set_number(AnimSetObj, "linked_animation", it_sets->second.linked_animation);

				JSON_Value* key_arr_val = json_value_init_array();
				JSON_Array* key_arr = json_value_get_array(key_arr_val);
				for (auto it_keys = it_sets->second.AnimEvts.begin(); it_keys != it_sets->second.AnimEvts.end(); ++it_keys)
				{
					JSON_Value* key_val = json_value_init_object();
					JSON_Object* key_obj = json_value_get_object(key_val);
					json_object_set_number(key_obj, "keyframe", it_keys->first);

					JSON_Value* comp_arr_val = json_value_init_array();
					JSON_Array* comp_arr = json_value_get_array(comp_arr_val);
					for (auto it_comps = it_keys->second.begin(); it_comps != it_keys->second.end(); ++it_comps)
					{
						JSON_Value* comp_val = json_value_init_object();
						JSON_Object* comp_obj = json_value_get_object(comp_val);
						json_object_set_number(comp_obj, "uuid", it_comps->first);

						JSON_Value* event_arr_val = json_value_init_array();
						JSON_Array* event_arr = json_value_get_array(event_arr_val);
						for (auto it_events = it_comps->second.begin(); it_events != it_comps->second.end(); ++it_events)
						{
							JSON_Value* event_val = json_value_init_object();
							JSON_Object* event_obj = json_value_get_object(event_val);
							json_object_set_number(event_obj, "event", it_events->first);
							// Now the value required should be save but currently not using it
							// When values are available for anim a switch for the value
							// adn depending on type of event and module should be saved
							
							json_array_append_value(event_arr, event_val);
						}
						json_object_set_value(comp_obj, "event_arr", event_arr_val);
						json_array_append_value(comp_arr, comp_val);
					}
					json_object_set_value(key_obj, "comp_arr", comp_arr_val);
					json_array_append_value(key_arr, key_val);
				}
				json_object_set_value(AnimSetObj,"key_arr", key_arr_val);
				json_array_append_value(AnimSetArr, AnimSetVal);
			}
			json_object_set_value(config, "AnimArr", AnimList);
		}
	}
}

bool ComponentBone::Update(float dt)
{
	float3 size = { 0.05f, 0.05f, 0.05f };
	float3 pos = ((ComponentTransform*)parent->getComponent(TRANSFORM))->global->getPosition();
	App->renderer3D->DirectDrawCube(size, pos);

	return true;
}

void ComponentBone::ProcessCompAnimations(const uint anim_uuid, const int frame)
{
	if (last_frame != frame)
	{
		auto get_AnimSet = AnimSets.find(anim_uuid);
		if (get_AnimSet != AnimSets.end())
		{
			auto get_key = get_AnimSet->second.AnimEvts.find(frame);
			if (get_key != get_AnimSet->second.AnimEvts.end())
			{
				std::list<Component*> components;
				getParent()->getComponents(components);
				for (auto it_uuids = get_key->second.begin(); it_uuids != get_key->second.end(); ++it_uuids)
				{
					for (auto it_comp = components.begin(); it_comp != components.end(); ++it_comp)
					{
						if (it_uuids->first == it_comp._Ptr->_Myval->getUUID())
							it_comp._Ptr->_Myval->ProcessAnimationEvents(it_uuids->second);
					}
				}
			}
		}
		last_frame = frame;
	}
}

bool ComponentBone::DrawInspector(int id)
{
	if (ImGui::CollapsingHeader("Bone"))
	{

	}
	return true;
}


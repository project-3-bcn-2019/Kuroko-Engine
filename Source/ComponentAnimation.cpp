#include "ComponentAnimation.h"
#include "Application.h"
#include "Applog.h"
#include "ModuleResourcesManager.h"
#include "ResourceAnimation.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ModuleTimeManager.h"

ComponentAnimation::ComponentAnimation(JSON_Object* deff, GameObject* parent): Component(parent, ANIMATION)
{
	loop = json_object_get_boolean(deff, "loop");
	speed = json_object_get_number(deff, "speed");

	const char* parent3dobject = json_object_get_string(deff, "Parent3dObject");
	if (parent3dobject) // Means that is being loaded from a scene
		animation_resource_uuid = App->resources->getAnimationResourceUuid(parent3dobject, json_object_get_string(deff, "animation_name"));
	else // Means it is being loaded from a 3dObject binary
		animation_resource_uuid = json_object_get_number(deff, "animation_resource_uuid");

	// Components Animations
	own_ticks = json_object_get_number(deff, "own_ticks");

	if (own_ticks != -1)
	{
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

			ComponentAnimations.insert(push_comp_anim);
		}
	}
	App->resources->assignResource(animation_resource_uuid);
}

ComponentAnimation::~ComponentAnimation()
{
}

bool ComponentAnimation::Update(float dt)
{
	if (App->time->getGameState() != GameState::PLAYING)
		return true;

	ResourceAnimation* anim = (ResourceAnimation*)App->resources->getResource(animation_resource_uuid);
	if (anim != nullptr)
	{
		if (bones.size() == 0) //If empty try to fill it
		{
			setAnimationResource(animation_resource_uuid);
			app_log->AddLog("Trying to fill component animation with bones");
		}

		//if (!TestPause)
		animTime += dt * speed;

		if (animTime > anim->getDuration() && loop)
		{
			animTime -= anim->getDuration();
		}

		for (int i = 0; i < anim->numBones; ++i)
		{
			if (bones.find(i) == bones.end())
				continue;

			GameObject* GO = App->scene->getGameObject(bones[i]);
			if (GO != nullptr)
			{
				ComponentTransform* transform = (ComponentTransform*)GO->getComponent(TRANSFORM);
				if (anim->boneTransformations[i].calcCurrentIndex(animTime*anim->ticksXsecond, false))
				{
					anim->boneTransformations[i].calcTransfrom(animTime*anim->ticksXsecond, true);
					float4x4 local = anim->boneTransformations[i].lastTransform;
					float3 pos, scale;
					Quat rot;
					local.Decompose(pos, rot, scale);
					transform->local->Set(pos, rot, scale);
				}
			}
		}
	}

	return true;
}

void ComponentAnimation::setAnimationResource(uint uuid)
{
	animation_resource_uuid = uuid;
	ResourceAnimation* anim = (ResourceAnimation*)App->resources->getResource(uuid);
	if (anim != nullptr)
	{
		for (int i = 0; i < anim->numBones; ++i)
		{
			GameObject* GO = parent->getChild(anim->boneTransformations[i].NodeName.c_str());
			if (GO != nullptr)
			{
				bones[i] = GO->getUUID();
			}
		}
	}
}

void ComponentAnimation::Save(JSON_Object * config)
{
	json_object_set_string(config, "type", "animation");

	json_object_set_boolean(config, "loop", loop);
	json_object_set_number(config, "speed", speed);

	if (animation_resource_uuid != 0)
	{
		ResourceAnimation* res_anim = (ResourceAnimation*)App->resources->getResource(animation_resource_uuid);
		if (res_anim)
		{
			json_object_set_string(config, "animation_name", res_anim->asset.c_str());
			json_object_set_string(config, "Parent3dObject", res_anim->Parent3dObject.c_str());
		}
		else
		{
		json_object_set_string(config, "animation_name", "missing reference");
		json_object_set_string(config, "Parent3dObject", "missing reference");
		}
	}

	// Component Animation
	json_object_set_number(config, "own_ticks", own_ticks);

	JSON_Value* comp_arr_val = json_value_init_array();
	JSON_Array* comp_arr = json_value_get_array(comp_arr_val);

	for (auto it = ComponentAnimations.begin(); it != ComponentAnimations.end(); ++it)
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

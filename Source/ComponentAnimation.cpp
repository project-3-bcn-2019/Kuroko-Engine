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
	if (App->is_game && !App->debug_game)
	{
		animation_resource_uuid = App->resources->getResourceUuid(json_object_get_string(deff, "animation_name"), R_ANIMATION);
	}
	else if (parent3dobject) // Means that is being loaded from a scene
		animation_resource_uuid = App->resources->getAnimationResourceUuid(parent3dobject, json_object_get_string(deff, "animation_name"));
	else // Means it is being loaded from a 3dObject binary
		animation_resource_uuid = json_object_get_number(deff, "animation_resource_uuid");

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
		if (!paused)
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
					anim->boneTransformations[i].calcTransfrom(animTime*anim->ticksXsecond, false);
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
	App->resources->deasignResource(animation_resource_uuid);
	animation_resource_uuid = uuid;
	App->resources->assignResource(animation_resource_uuid);
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
}
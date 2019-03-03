#include "ComponentBone.h"
#include "Application.h"
#include "ModuleResourcesManager.h"
#include "ResourceBone.h"
#include "ModuleRenderer3D.h"
#include "GameObject.h"
#include "ComponentTransform.h"

ComponentBone::ComponentBone(JSON_Object* deff, GameObject* parent) : Component(parent, BONE)
{
	const char* parent3dobject = json_object_get_string(deff, "Parent3dObject");
	if (parent3dobject) // Means that is being loaded from a scene
		bone_resource_uuid = App->resources->getBoneResourceUuid(parent3dobject, json_object_get_string(deff, "bone_name"));
	else // Means it is being loaded from a 3dObject binary
		bone_resource_uuid = json_object_get_number(deff, "bone_resource_uuid");

	App->resources->assignResource(bone_resource_uuid);
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
	}
}

bool ComponentBone::Update(float dt)
{
	float3 size = { 0.05f, 0.05f, 0.05f };
	float3 pos = ((ComponentTransform*)parent->getComponent(TRANSFORM))->global->getPosition();
	App->renderer3D->DirectDrawCube(size, pos);

	return true;
}

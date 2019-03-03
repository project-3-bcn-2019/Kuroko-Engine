#include "ComponentTransform.h"
#include "GameObject.h"
#include "Transform.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleDebug.h"

#include "glew-2.1.0\include\GL\glew.h"

ComponentTransform::ComponentTransform(GameObject* parent, const Quat& rot, const float3& pos, const float3& scl) : Component(parent, TRANSFORM)
{
	local = new Transform();
	global = new Transform();

	local->setPosition(pos);
	local->setRotation(rot);
	local->setScale(scl);
	local->CalculateMatrix();

	LocalToGlobal();
}

ComponentTransform::ComponentTransform(GameObject* parent, const float3& euler_axis, const float3& pos, const float3& scl) : Component(parent, TRANSFORM)
{
	local = new Transform();
	global = new Transform();

	local->setPosition(pos);
	local->setRotation(Quat::FromEulerXYZ(euler_axis.x, euler_axis.y, euler_axis.z));
	local->setScale(scl);
	local->CalculateMatrix();

	LocalToGlobal();
}

ComponentTransform::ComponentTransform(GameObject* parent, const ComponentTransform& transform) : Component(parent, TRANSFORM)
{
	local = new Transform(*transform.local);
	global = new Transform(*transform.global);
}

ComponentTransform::ComponentTransform(JSON_Object* deff, GameObject* parent): Component(parent, TRANSFORM){
	local = new Transform(json_object_get_object(deff, "local"));
	global = new Transform(json_object_get_object(deff, "global"));
}

ComponentTransform::ComponentTransform(GameObject* parent) : Component(parent, TRANSFORM)
{
	local = new Transform();
	global = new Transform();
}

ComponentTransform::~ComponentTransform()
{
	if (local) delete local;
	if (global) delete global;
}

bool ComponentTransform::Update(float dt)
{
	if (mode == GLOBAL)
	{
		global->CalculateMatrix();
		GlobalToLocal();
	}
	else
	{
		local->CalculateMatrix();
		LocalToGlobal();
	}


	return true;
}

Transform* ComponentTransform::getInheritedTransform()
{
	if (GameObject* parent_obj = getParent()->getParent())
		return ((ComponentTransform*)parent_obj->getComponent(TRANSFORM))->global;
	else 
		return nullptr;
}

void ComponentTransform::GlobalToLocal()
{
	if (Transform* inh_transform = getInheritedTransform())
	{
		local->setPosition(inh_transform->getRotation().Inverted() * (global->getPosition() - (inh_transform->getRotation() * local->getRotation() * float3(getParent()->own_centroid.x *  inh_transform->getScale().x, getParent()->own_centroid.y *  inh_transform->getScale().y, getParent()->own_centroid.z *  inh_transform->getScale().z))) - inh_transform->getPosition());
		local->setRotation(inh_transform->getRotation().Inverted() * global->getRotation());
		local->setScale(global->getScale().Div(inh_transform->getScale()));
	}
	else
		local->Set(global->getPosition(), global->getRotation(), global->getScale());

	local->CalculateMatrix();
}

void ComponentTransform::LocalToGlobal()
{
	if (Transform* inh_transform = getInheritedTransform())
	{
		global->setPosition(inh_transform->getRotation() * local->getPosition() + inh_transform->getPosition() + (inh_transform->getRotation() * local->getRotation() * float3(getParent()->own_centroid.x *  inh_transform->getScale().x, getParent()->own_centroid.y *  inh_transform->getScale().y, getParent()->own_centroid.z *  inh_transform->getScale().z)));
		global->setRotation(inh_transform->getRotation() * local->getRotation());
		global->setScale(local->getScale().Mul(inh_transform->getScale()));
	}
	else
		global->Set(local->getPosition(), local->getRotation(), local->getScale());

	global->CalculateMatrix();
}

void ComponentTransform::Draw() const
{
	if (draw_axis)
		App->debug->directDrawAxis(global->getPosition(), global->getRotation());
}

void ComponentTransform::Save(JSON_Object* config) {
	// Component has an object called transform, which has the two transforms as attributes
	JSON_Value* local_trans = json_value_init_object();
	JSON_Value* global_trans = json_value_init_object();
	// Set component type
	json_object_set_string(config, "type", "transform");

	local->Save(json_object(local_trans));
	global->Save(json_object(global_trans));

	json_object_set_value(config, "local", local_trans);
	json_object_set_value(config, "global", global_trans);
}


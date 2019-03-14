#include "ComponentTrigger.h"
#include "GameObject.h"
//#include "ComponentCamera.h"
////#include "ModuleSceneImGui.h"
//#include "Globals.h"
//#include "Camera.h"
//#include "Application.h"
#include "PhysBody.h"
#include "Transform.h"
#include "ComponentTransform.h"
#include "ComponentAABB.h"
#include "Application.h"
#include "ModulePhysics3D.h"
#include "ModuleInput.h"
#include "ModuleTimeManager.h"

#include "SDL/include/SDL_opengl.h"
#include "ImGui/imgui.h"

ComponentTrigger::ComponentTrigger(GameObject * _parent, collision_shape _shape) :Component(_parent, TRIGGER)
{

	shape = _shape;
	body = App->physics->AddTrigger(this,shape);
	body->setUserPointer(this);

	//body = App->physics->AddTrigger(this, shape);
	//body->setUserPointer(this);
}

ComponentTrigger::ComponentTrigger(JSON_Object * deff, GameObject * parent) :Component(parent, TRIGGER)
{
	JSON_Object* p = json_object_get_object(deff, "pos");
	JSON_Object* r = json_object_get_object(deff, "rot");
	JSON_Object* s = json_object_get_object(deff, "scale");
	JSON_Object* data = json_object_get_object(deff, "data");


	offset_pos = float3(json_object_get_number(p, "offset_pos_x"), json_object_get_number(p, "offset_pos_y"), json_object_get_number(p, "offset_pos_z"));
	offset_rot = float3(json_object_get_number(r, "offset_rot_x"), json_object_get_number(r, "offset_rot_y"), json_object_get_number(r, "offset_rot_z"));
	offset_scale = float3(json_object_get_number(s, "offset_scale_x"), json_object_get_number(s, "offset_scale_y"), json_object_get_number(s, "offset_scale_z"));

	shape = (collision_shape)(int)json_object_get_number(data, "shape");

	body = App->physics->AddTrigger(this, shape);
	body->setUserPointer(this);
}

bool ComponentTrigger::Update(float dt)
{



	UpdatePhysicsFromTransforms();

	for (int i = 0; i < body->getNumOverlappingObjects(); i++)
	{
		// Dynamic cast to make sure its a rigid body
		btRigidBody *pRigidBody = dynamic_cast<btRigidBody *>(body->getOverlappingObject(i));
		if (pRigidBody)
		{
			printf("lul");
		}
	}

	colliding.clear();
	std::list<Collision> col_list;
	App->physics->GetCollisionsFromObject(col_list, getParent());

	for (std::list<Collision>::iterator item = col_list.begin(); item != col_list.end(); ++item)
	{
		colliding.push_back((*item).B);
	}


	return true;
}

void ComponentTrigger::Draw() const
{


}

bool ComponentTrigger::DrawInspector(int id)
{
	if (ImGui::CollapsingHeader("Trigger"))
	{
		ImGui::TextWrapped("Drag the parameters to change them, or ctrl+click on one of them to set it's value");
		//position
		ImGui::Text("Offset:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##p x", &offset_pos.x, 0.01f, 0.0f, 0.0f, "%.02f");

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##p y", &offset_pos.y, 0.01f, 0.0f, 0.0f, "%.02f");

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##p z", &offset_pos.z, 0.01f, 0.0f, 0.0f, "%.02f");

		//rotation
		ImGui::Text("Rotation:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##r x", &offset_rot.x, 0.2f, -180.0f, 180.0f, "%.02f");

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##r y", &offset_rot.y, 0.2f, -180.0f, 180.0f, "%.02f");

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##r z", &offset_rot.z, 0.2f, -180.0f, 180.0f, "%.02f");

		//scale
		ImGui::Text("   Scale:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##s x", &offset_scale.x, 0.01f, -1000.0f, 1000.0f, "%.02f");

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##s y", &offset_scale.y, 0.01f, -1000.0f, 1000.0f, "%.02f");

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##s z", &offset_scale.z, 0.01f, -1000.0f, 1000.0f, "%.02f");


		if (ImGui::Button("Change_shape"))
		{
			App->physics->ChangeShape(this);
		}

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.f, 0.f, 0.f, 1.f)); ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.f, 0.2f, 0.f, 1.f));
		if (ImGui::Button("Remove##Remove trigger")) {
			ImGui::PopStyleColor(); ImGui::PopStyleColor();
			return false;
		}
		ImGui::PopStyleColor(); ImGui::PopStyleColor();
	}

	return true;
}

void ComponentTrigger::Save(JSON_Object* config)
{
	json_object_set_string(config, "type", "trigger");

	JSON_Value* data = json_value_init_object();
	json_object_set_number(json_object(data), "shape", shape);

	json_object_set_value(config, "data", data);

	JSON_Value* pos = json_value_init_object();
	json_object_set_number(json_object(pos), "offset_pos_x", offset_pos.x);
	json_object_set_number(json_object(pos), "offset_pos_y", offset_pos.y);
	json_object_set_number(json_object(pos), "offset_pos_z", offset_pos.z);

	json_object_set_value(config, "pos", pos);

	JSON_Value* rot = json_value_init_object();
	json_object_set_number(json_object(rot), "offset_rot_x", offset_rot.x);
	json_object_set_number(json_object(rot), "offset_rot_y", offset_rot.y);
	json_object_set_number(json_object(rot), "offset_rot_z", offset_rot.z);

	json_object_set_value(config, "rot", rot);

	JSON_Value* scale = json_value_init_object();
	json_object_set_number(json_object(scale), "offset_scale_x", offset_scale.x);
	json_object_set_number(json_object(scale), "offset_scale_y", offset_scale.y);
	json_object_set_number(json_object(scale), "offset_scale_z", offset_scale.z);

	json_object_set_value(config, "scale", scale);



}

void ComponentTrigger::UpdateTransformsFromPhysics()
{
	GameObject* obj = getParent();
	if (obj != nullptr)
	{
		ComponentTransform* transform = (ComponentTransform*)obj->getComponent(TRANSFORM);
		if (transform != nullptr)
		{

			btTransform t = body->getWorldTransform();

			body->getCollisionShape()->setLocalScaling(btVector3(offset_scale.x, offset_scale.y, offset_scale.z));

			float3 obj_pos = float3(t.getOrigin().getX() - offset_pos.x, t.getOrigin().getY() - offset_pos.y, t.getOrigin().getZ() - offset_pos.z);

			Quat q = Quat::FromEulerXYZ(offset_rot.x, offset_rot.y, offset_rot.z);
			Quat obj_rot = Quat(t.getRotation().getX() - q.x, t.getRotation().getY() - q.y, t.getRotation().getZ() - q.z, t.getRotation().getW() - q.w);

			transform->local->setPosition(obj_pos);
			transform->local->setRotation(obj_rot);

		}
	}
}

void ComponentTrigger::UpdatePhysicsFromTransforms()
{

	GameObject* obj = getParent();
	if (obj != nullptr)
	{
		ComponentTransform* transform = (ComponentTransform*)obj->getComponent(TRANSFORM);
		if (transform != nullptr)
		{

			btTransform t;
			t.setIdentity();

			body->getCollisionShape()->setLocalScaling(btVector3(offset_scale.x, offset_scale.y, offset_scale.z));

			btVector3 obj_pos = btVector3(transform->local->getPosition().x + offset_pos.x, transform->local->getPosition().y + offset_pos.y, transform->local->getPosition().z + offset_pos.z);
			Quat q = Quat::FromEulerXYZ(offset_rot.x, offset_rot.y, offset_rot.z);
			btQuaternion obj_rot = btQuaternion(transform->local->getRotation().x + q.x, transform->local->getRotation().y + q.y, transform->local->getRotation().z + q.z, transform->local->getRotation().w + q.w);

			t.setOrigin(obj_pos);
			t.setRotation(obj_rot);

			body->setWorldTransform(t);

		}
	}
}

//void ComponentPhysics::ToggleStatic()
//{
//	if (is_environment)
//	{
//		body->GetRigidBody()->setCollisionFlags(btRigidBody::CollisionFlags::CF_KINEMATIC_OBJECT);
//		is_environment = false;
//	}
//	else
//	{
//		body->GetRigidBody()->setCollisionFlags(btRigidBody::CollisionFlags::CF_STATIC_OBJECT);
//		is_environment = true;
//	}
//}

ComponentTrigger::~ComponentTrigger()
{
	App->physics->DeleteTrigger(this);

};
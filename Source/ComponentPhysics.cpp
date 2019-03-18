#include "ComponentPhysics.h"
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


ComponentPhysics::ComponentPhysics(GameObject * _parent, collision_shape _shape, bool _is_environment) :Component(_parent, PHYSICS)
{
	body = App->physics->AddBody(this,_shape, _is_environment);
	body->SetUser(this);

	shape = _shape;
	is_environment = _is_environment;

	mass = 1;

	if (is_environment)
	{
		body->GetRigidBody()->setCollisionFlags(btRigidBody::CollisionFlags::CF_STATIC_OBJECT);
		body->GetRigidBody()->setMassProps(0, btVector3(0, 0, 0));
	}
	else
	{
		body->GetRigidBody()->setCollisionFlags(btRigidBody::CollisionFlags::CF_CHARACTER_OBJECT);
		body->GetRigidBody()->setMassProps(1, btVector3(0, 0, 0));

	}

}

ComponentPhysics::ComponentPhysics(JSON_Object * deff, GameObject * parent) :Component(parent, PHYSICS)
{
	is_active = json_object_get_boolean(deff, "active");

	JSON_Object* p = json_object_get_object(deff, "pos");
	JSON_Object* r = json_object_get_object(deff, "rot");
	JSON_Object* s = json_object_get_object(deff, "scale");
	JSON_Object* size = json_object_get_object(deff, "size");

	JSON_Object* data = json_object_get_object(deff, "data");

	offset_pos = float3(json_object_get_number(p, "offset_pos_x"), json_object_get_number(p, "offset_pos_y"), json_object_get_number(p, "offset_pos_z"));
	offset_rot = float3(json_object_get_number(r, "offset_rot_x"), json_object_get_number(r, "offset_rot_y"), json_object_get_number(r, "offset_rot_z"));
	offset_scale = float3(json_object_get_number(s, "offset_scale_x"), json_object_get_number(s, "offset_scale_y"), json_object_get_number(s, "offset_scale_z"));

	collider_size = float3(json_object_get_number(size, "size_x"), json_object_get_number(size, "size_y"), json_object_get_number(size, "size_z"));

	mass = json_object_get_number(data,"mass");
	if (mass == 0)
		mass = 1;

	shape = (collision_shape)(int)json_object_get_number(data, "shape");
	is_environment = json_object_get_boolean(data, "is_environment");

	body = App->physics->AddBody(this, shape, is_environment,collider_size);
	body->SetUser(this);

	if (is_environment)
	{
		body->GetRigidBody()->setCollisionFlags(btRigidBody::CollisionFlags::CF_STATIC_OBJECT);
		body->GetRigidBody()->setMassProps(0, btVector3(0, 0, 0));
	}
	else
	{
		body->GetRigidBody()->setCollisionFlags(btRigidBody::CollisionFlags::CF_CHARACTER_OBJECT);
		body->GetRigidBody()->setMassProps(mass, btVector3(0, 0, 0));

	}
}

bool ComponentPhysics::Update(float dt)
{
	//TODO
	//Gather the pointer with the transform matrix
	
	//float* transform_matrix = new float[16];
	//transform_matrix = ((ComponentTransform*)parent->getComponent(TRANSFORM))->global->getMatrix().ptr();

	//body->SetTransform(transform_matrix);

/*
	GameObject* obj = getParent();
	if (obj != nullptr)
	{


		ComponentTransform* transform = (ComponentTransform*)obj->getComponent(TRANSFORM);
		if (transform != nullptr)
		{
			btTransform t;
			float4x4 fina;

			fina = float4x4::FromTRS(float3(0, 0, 0), Quat::identity, transform->global->getScale());

			fina.Transpose();

			Quat newquat = transform->global->getRotation();
			newquat.Inverse();
			float4x4 rot_mat = newquat.ToFloat4x4();

			fina = /*fina * *rot_mat;

			//fina = fina * float4x4::FromQuat(transform->global->getRotation());
			//fina.Translate(transform->global->getPosition());

			t.setFromOpenGLMatrix(fina.ptr());

			t.setOrigin(btVector3(transform->global->getPosition().x, transform->global->getPosition().y, transform->global->getPosition().z));

			body->GetRigidBody()->getCollisionShape()->setLocalScaling(btVector3(transform->global->getScale().x, transform->global->getScale().y, transform->global->getScale().z));

			body->GetRigidBody->setWorldTransform(t);
		}
	}*/

	//the objective is to get the transform from the physics object and apply it to the obb and then to the object in the engine
	if (App->time->getGameState() == GameState::PLAYING)
	{
		UpdateTransformsFromPhysics();
	}
	else
	{
		UpdatePhysicsFromTransforms();
		body->SetSpeed(0, 0, 0);
	}

	if (is_environment)
	{
		body->SetSpeed(0, 0, 0);
	}

	//btTransform t;
	//t = body->GetRigidBody()->getWorldTransform();

	//float4x4 m;
	//t.getOpenGLMatrix(m.ptr());


	//bounding_box. = { transform->getScale().x, transform->getScale().y, transform->getScale().z };/*forgive me pls*/


	colliding.clear();
	std::list<Collision> col_list;
	App->physics->GetCollisionsFromObject(col_list, getParent());

	for (std::list<Collision>::iterator item = col_list.begin(); item != col_list.end(); ++item)
	{
		colliding.push_back((*item).B);
	}


	return true;
}

void ComponentPhysics::Draw() const
{


}

bool ComponentPhysics::DrawInspector(int id)
{
	if (ImGui::CollapsingHeader("Collider"))
	{
		ImGui::TextWrapped("Drag the parameters to change them, or ctrl+click on one of them to set it's value");
		
		bool toggle_static = is_environment;

		float3 rot_deg = offset_rot * RADTODEG;

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
		ImGui::DragFloat("##r x", &rot_deg.x, 0.2f, -180.0f, 180.0f, "%.02f");

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##r y", &rot_deg.y, 0.2f, -180.0f, 180.0f, "%.02f");

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##r z", &rot_deg.z, 0.2f, -180.0f, 180.0f, "%.02f");

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

		ImGui::Text("   Mass");
		ImGui::DragFloat("##s Mass", &mass, 0.01f, 0.1f, 1000.0f, "%.02f");
		if (ImGui::Button("Update Mass"))
		{
			SetMass(mass);
		}

		offset_rot = DEGTORAD * rot_deg;

		ImGui::Checkbox("is environment", &toggle_static);
		if (toggle_static != is_environment)
		{
			SetStatic(toggle_static);
		}

		if (ImGui::Button("Change shape"))
		{
			App->physics->ChangeShape(this);
		}

		ImGui::SameLine();
		if (ImGui::Button("Adapt to OBB"))
		{
			App->physics->AdaptToOBB(this);
		}

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.f, 0.f, 0.f, 1.f)); ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.f, 0.2f, 0.f, 1.f));
		if (ImGui::Button("Remove##Remove physics")) {
			ImGui::PopStyleColor(); ImGui::PopStyleColor();
			return false;
		}
		ImGui::PopStyleColor(); ImGui::PopStyleColor();
	}

	return true;
}

void ComponentPhysics::Save(JSON_Object* config)
{
	json_object_set_string(config, "type", "physics");
	json_object_set_boolean(config, "active", is_active);

	JSON_Value* data = json_value_init_object();
	json_object_set_number(json_object(data), "shape", shape);
	json_object_set_boolean(json_object(data), "is_environment", is_environment);
	json_object_set_number(json_object(data), "mass", mass);

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

	JSON_Value* size = json_value_init_object();
	json_object_set_number(json_object(size), "size_x", collider_size.x);
	json_object_set_number(json_object(size), "size_y", collider_size.y);
	json_object_set_number(json_object(size), "size_z", collider_size.z);

	json_object_set_value(config, "size", size);


}

void ComponentPhysics::UpdateTransformsFromPhysics()
{
	GameObject* obj = getParent();
	if (obj != nullptr)
	{
		ComponentTransform* transform = (ComponentTransform*)obj->getComponent(TRANSFORM);
		if (transform != nullptr)
		{

			btTransform t = body->GetRigidBody()->getWorldTransform();

			body->GetRigidBody()->getCollisionShape()->setLocalScaling(btVector3(offset_scale.x, offset_scale.y, offset_scale.z));

			float3 obj_pos = float3(t.getOrigin().getX() - offset_pos.x, t.getOrigin().getY() - offset_pos.y, t.getOrigin().getZ() - offset_pos.z);

			Quat q = Quat::FromEulerXYZ(offset_rot.x,offset_rot.y,offset_rot.z);
			Quat obj_rot = Quat(t.getRotation().getX()-q.x, t.getRotation().getY()-q.y, t.getRotation().getZ()-q.z, t.getRotation().getW()-q.w);

			transform->local->setPosition(obj_pos);
			transform->local->setRotation(obj_rot);

		}
	}
}

void ComponentPhysics::UpdatePhysicsFromTransforms()
{

	GameObject* obj = getParent();
	if (obj != nullptr)
	{
		ComponentTransform* transform = (ComponentTransform*)obj->getComponent(TRANSFORM);
		if (transform != nullptr)
		{

			btTransform t;
			t.setIdentity();
			
			body->GetRigidBody()->getCollisionShape()->setLocalScaling(btVector3(offset_scale.x,offset_scale.y,offset_scale.z));

			btVector3 obj_pos = btVector3(transform->local->getPosition().x + offset_pos.x, transform->local->getPosition().y + offset_pos.y, transform->local->getPosition().z + offset_pos.z);
			//Quat q = Quat::FromEulerXYZ(RadToDeg(offset_rot.x), RadToDeg(offset_rot.y), RadToDeg(offset_rot.z));
			Quat q = Quat::FromEulerXYZ(offset_rot.x+ transform->global->getRotation().ToEulerXYZ().x, offset_rot.y + transform->global->getRotation().ToEulerXYZ().y, offset_rot.z + transform->global->getRotation().ToEulerXYZ().z);

			//Quat final_quat = transform->local->getRotation() + q;

			//btQuaternion obj_rot = btQuaternion(transform->local->getRotation().x + q.x, transform->local->getRotation().y+q.y, transform->local->getRotation().z+q.z, transform->local->getRotation().w+q.w);
			btQuaternion obj_rot = btQuaternion(q.x, q.y, q.z, q.w);

			t.setOrigin(obj_pos);
			t.setRotation(obj_rot);

			body->GetRigidBody()->setWorldTransform(t);

		}
	}
}

void ComponentPhysics::SetSpeed(float x, float y, float z)//@LUCAS ESTA ES LA FUNCION QUE TIENES QUE LLAMAR PARA PONERLE LA VELOCIDAD!!
{
	body->SetSpeed(x, y, z);
}

void ComponentPhysics::SetPosition(float x, float y, float z)
{
	body->SetPos(x, y, z);

}

void ComponentPhysics::SetDamping(float new_value_linear, float new_value_angular)
{

	body->GetRigidBody()->setDamping(new_value_linear, new_value_angular);

}

void ComponentPhysics::SetMass(float new_value)
{
	mass = new_value;
	App->physics->world->removeRigidBody(body->GetRigidBody());
	body->GetRigidBody()->setMassProps(new_value, btVector3(0, 0, 0));
	App->physics->world->addRigidBody(body->GetRigidBody());

}

void ComponentPhysics::SetStatic(bool is_static)
{
	if (is_static)
	{
		body->GetRigidBody()->setCollisionFlags(btRigidBody::CollisionFlags::CF_STATIC_OBJECT);
		App->physics->world->removeRigidBody(body->GetRigidBody());
		body->GetRigidBody()->setMassProps(0, btVector3(0, 0, 0));
		App->physics->world->addRigidBody(body->GetRigidBody());

		is_environment = true;
	}
	else
	{
		body->GetRigidBody()->setCollisionFlags(btRigidBody::CollisionFlags::CF_CHARACTER_OBJECT);
		App->physics->world->removeRigidBody(body->GetRigidBody());
		body->GetRigidBody()->setMassProps(mass, btVector3(0, 0, 0));
		App->physics->world->addRigidBody(body->GetRigidBody());

		is_environment = false;
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

ComponentPhysics::~ComponentPhysics()
{

	App->physics->DeleteBody(body);

};
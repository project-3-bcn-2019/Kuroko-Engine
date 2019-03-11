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


ComponentPhysics::ComponentPhysics(GameObject * _parent, collision_shape _shape, bool _is_environment) :Component(_parent, PHYSICS)
{
	body = App->physics->AddBody(this,_shape, _is_environment);
	body->SetUser(this);

	shape = _shape;
	is_environment = _is_environment;

}

ComponentPhysics::ComponentPhysics(JSON_Object * deff, GameObject * parent) :Component(parent, PHYSICS)
{
	JSON_Object* p = json_object_get_object(deff, "pos");
	JSON_Object* r = json_object_get_object(deff, "rot");
	JSON_Object* s = json_object_get_object(deff, "scale");
	JSON_Object* data = json_object_get_object(deff, "data");


	offset_pos = float3(json_object_get_number(p, "offset_pos_x"), json_object_get_number(p, "offset_pos_y"), json_object_get_number(p, "offset_pos_z"));
	offset_rot = float3(json_object_get_number(r, "offset_rot_x"), json_object_get_number(r, "offset_rot_y"), json_object_get_number(r, "offset_rot_z"));
	offset_scale = float3(json_object_get_number(s, "offset_scale_x"), json_object_get_number(s, "offset_scale_y"), json_object_get_number(s, "offset_scale_z"));

	shape = (collision_shape)(int)json_object_get_number(data, "shape");
	is_environment = json_object_get_boolean(data, "is_environment");

	body = App->physics->AddBody(this, shape, is_environment);
	body->SetUser(this);

	if (is_environment)
	{
		body->GetRigidBody()->setCollisionFlags(btRigidBody::CollisionFlags::CF_STATIC_OBJECT);
	}
	else
	{
		body->GetRigidBody()->setCollisionFlags(btRigidBody::CollisionFlags::CF_CHARACTER_OBJECT);
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

	if (App->input->GetKey(SDL_SCANCODE_G) == KEY_DOWN)
	{
		//body->GetRigidBody()->ap(btVector3(10,0,0));
		App->physics->change_shape(this);
	}

	if (App->time->getGameState() == GameState::PLAYING)
	{
		UpdateTransformsFromPhysics();
	}
	else
	{
		UpdatePhysicsFromTransforms();
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

void ComponentPhysics::Save(JSON_Object* config)
{
	json_object_set_string(config, "type", "physics");

	JSON_Value* data = json_value_init_object();
	json_object_set_number(json_object(data), "shape", shape);
	json_object_set_boolean(json_object(data), "is_environment", is_environment);

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
			Quat q = Quat::FromEulerXYZ(offset_rot.x, offset_rot.y, offset_rot.z);
			btQuaternion obj_rot = btQuaternion(transform->local->getRotation().x + q.x, transform->local->getRotation().y+q.y, transform->local->getRotation().z+q.z, transform->local->getRotation().w+q.w);

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

void ComponentPhysics::SetStatic(bool is_static)
{
	if (is_static)
	{
		body->GetRigidBody()->setCollisionFlags(btRigidBody::CollisionFlags::CF_STATIC_OBJECT);
		is_environment = true;
	}
	else
	{
		body->GetRigidBody()->setCollisionFlags(btRigidBody::CollisionFlags::CF_CHARACTER_OBJECT);
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
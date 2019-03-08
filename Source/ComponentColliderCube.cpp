#include "ComponentColliderCube.h"
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

#include "SDL/include/SDL_opengl.h"


ComponentColliderCube::ComponentColliderCube(GameObject * _parent) :Component(_parent, COLLIDER_CUBE)
{
	body = App->physics->AddBody(_parent);
	body->SetUser(this);

}

ComponentColliderCube::ComponentColliderCube(JSON_Object * deff, GameObject * parent) :Component(parent, COLLIDER_CUBE)
{
	JSON_Object* j = json_object_get_object(deff, "obb");

	int i = json_object_get_number(j, "sizex");

	((ComponentAABB*)getParent()->getComponent(C_AABB))->getOBB()->SetFrom(AABB(
		float3(-json_object_get_number(j, "sizex") / 2, -json_object_get_number(j, "sizey") / 2, -json_object_get_number(j, "sizez") / 2), 
		float3(json_object_get_number(j, "sizex") / 2, json_object_get_number(j, "sizey") / 2, json_object_get_number(j, "sizez") / 2)));

	getParent()->own_half_size = float3(json_object_get_number(j, "sizex") / 2, json_object_get_number(j, "sizey") / 2, json_object_get_number(j, "sizez") / 2);


	body = App->physics->AddBody(parent);
	body->SetUser(this);

}

void ComponentColliderCube::OnCollision(GameObject * A, GameObject * B)
{
	
}

bool ComponentColliderCube::Update(float dt)
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

	colliding.clear();
	std::list<Collision> col_list;
	App->physics->GetCollisionsFromObject(col_list, getParent());

	for (std::list<Collision>::iterator item = col_list.begin(); item != col_list.end(); ++item)
	{
		colliding.push_back((*item).B);
	}

	return true;
}

void ComponentColliderCube::Draw() const
{
	//ComponentAABB* aabb = ((ComponentAABB*)parent->getComponent(C_AABB));

	//float sx = aabb-> * scale.x * 0.25f;
	//float sy = dimensions.y * scale.y * 0.25f;
	//float sz = dimensions.z * scale.z * 0.25f;

	//glLineWidth(5.0f);
	//glBegin(GL_QUADS);

	//glNormal3f(0.0f, 0.0f, 1.0f);
	//glVertex3f(-sx, -sy, sz);
	//glVertex3f(sx, -sy, sz);
	//glVertex3f(sx, sy, sz);
	//glVertex3f(-sx, sy, sz);

	//glNormal3f(0.0f, 0.0f, -1.0f);
	//glVertex3f(sx, -sy, -sz);
	//glVertex3f(-sx, -sy, -sz);
	//glVertex3f(-sx, sy, -sz);
	//glVertex3f(sx, sy, -sz);

	//glNormal3f(1.0f, 0.0f, 0.0f);
	//glVertex3f(sx, -sy, sz);
	//glVertex3f(sx, -sy, -sz);
	//glVertex3f(sx, sy, -sz);
	//glVertex3f(sx, sy, sz);

	//glNormal3f(-1.0f, 0.0f, 0.0f);
	//glVertex3f(-sx, -sy, -sz);
	//glVertex3f(-sx, -sy, sz);
	//glVertex3f(-sx, sy, sz);
	//glVertex3f(-sx, sy, -sz);

	//glNormal3f(0.0f, 1.0f, 0.0f);
	//glVertex3f(-sx, sy, sz);
	//glVertex3f(sx, sy, sz);
	//glVertex3f(sx, sy, -sz);
	//glVertex3f(-sx, sy, -sz);

	//glNormal3f(0.0f, -1.0f, 0.0f);
	//glVertex3f(-sx, -sy, -sz);
	//glVertex3f(sx, -sy, -sz);
	//glVertex3f(sx, -sy, sz);
	//glVertex3f(-sx, -sy, sz);

	//glEnd();

}

//{
//	this->SetOwner(owner);
//	this->SetActive(true);
//	SetName("Component Collider");
//	type = ComponentType::COLLIDERCUBE;
//	PCube* aux_cube = new PCube();
//
//	float3 diagonal_aabb = { 5,5,5 };
//	float3 position = { 0,0,0 };
//
//
//	if (owner->HasMesh())
//	{
//		aux_cube->has_primitive_render = false;				
//		AABB aux_bb = owner->GetBB();
//		position = aux_bb.CenterPoint();
//		diagonal_aabb = aux_bb.Diagonal();
//		
//		
//	}
//		 
//	aux_cube->dimensions = diagonal_aabb;
//	aux_cube->SetPos(position.x, position.y, position.z);
//
//	if (owner->physbody == nullptr)
//	{
//		new PhysBody(owner, aux_cube);		
//	}
//	if (owner->HasMesh())
//	{
//		owner->physbody->mesh_ptr = owner->GetMesh();
//	}
//	Update();
//}
//
//ComponentColliderCube::ComponentColliderCube(GameObject * owner, PCube * pcube)
//{
//	this->SetOwner(owner);
//	this->SetActive(true);
//	SetName("Component Collider");
//	type = ComponentType::COLLIDERCUBE;
//	if (owner->physbody == nullptr)
//	{
//		new PhysBody(owner, pcube);
//	}
//
//}
//
//ComponentColliderCube::~ComponentColliderCube()
//{
//}
//
//bool ComponentColliderCube::Update()
//{	
//
//	//Gather the pointer with the transform matrix
//	float* transform_matrix = new float[16];
//	transform_matrix = owner->comp_transform->trans_matrix_g.ptr();
//
//	//Get the identity of the body
//	float current_pmatrix[16];
//	owner->physbody->GetTransform(current_pmatrix);
//
//	//Creating the final matrix where we will create the new base
//	for (int i = 0; i < 16; i++)
//	{
//		final_pmatrix[i] = current_pmatrix[i];
//	}
//
//	float3x3 rot = {
//		transform_matrix[0],transform_matrix[1],transform_matrix[2],
//		transform_matrix[4],transform_matrix[5],transform_matrix[6],
//		transform_matrix[8],transform_matrix[9],transform_matrix[10]
//	};
//
//	rot.Transpose();
//
//	//Relate both matrix rotation and inverting the rotation
//	final_pmatrix[0] = rot[0][0];		final_pmatrix[1] = rot[0][1];			final_pmatrix[2] = rot[0][2];
//	final_pmatrix[4] = rot[1][0];		final_pmatrix[5] = rot[1][1];			final_pmatrix[6] = rot[1][2];
//	final_pmatrix[8] = rot[2][0];		final_pmatrix[9] = rot[2][1];			final_pmatrix[10] = rot[2][2];
//	float3 position = { 0,0,0 };
//	if (owner->HasMesh())
//	{
//		//Relate both matrix translation
//		position = owner->GetBB().CenterPoint();
//		final_pmatrix[12] = position.x + center_offset[0];
//		final_pmatrix[13] = position.y + center_offset[1];
//		final_pmatrix[14] = position.z + center_offset[2];
//	}
//	else
//	{
//		//Relate both matrix translation
//		final_pmatrix[12] = transform_matrix[3] + center_offset[0];
//		final_pmatrix[13] = transform_matrix[7] + center_offset[1];
//		final_pmatrix[14] = transform_matrix[11] + center_offset[2];
//	}
//
//	//Add the result on the object
//	owner->physbody->SetTransform(final_pmatrix);
//	
//
//	return false;
//}
//
//void ComponentColliderCube::DrawInspectorInfo()
//{
//
//	
//	
//	if (ImGui::DragFloat3("Center##collider", center_offset, 0.1f, -INFINITY, INFINITY)) 
//	{
//		Update();
//	}
//
//
//	if (ImGui::DragFloat3("Dimensions##collider", dimensions_component.ptr(), 0.01f, 1, 10))
//	{
//		btVector3 scaling = { dimensions_component.x,dimensions_component.y,dimensions_component.z };
//		owner->physbody->GetRigidBody()->getCollisionShape()->setLocalScaling(scaling);
//		owner->physbody->primitive_ptr->scale = dimensions_component;
//		
//	}
//}
//
//void ComponentColliderCube::UpdateTransform()
//{
//	if (HasOwner())
//	{
//		GameObject* owner = GetOwner();
//		owner->physbody->SetTransform((float*)&owner->comp_transform->trans_matrix_g);			
//	}
//}
//
//bool ComponentColliderCube::HasMoved()
//{
//	bool ret = false;
//
//	if (!IsBulletStatic())
//	{
//		//PAU WiP
//	}
//
//	return ret;
//}
//bool ComponentColliderCube::IsBulletStatic()
//{
//	bool ret = owner->physbody->GetRigidBody()->isStaticObject();
//	return ret;
//}
//
void ComponentColliderCube::Save(JSON_Object* config)
{
	json_object_set_string(config, "type", "collider_cube");
	((ComponentAABB*)getParent()->getComponent(C_AABB))->getOBB()->Size();

	JSON_Value* obb = json_value_init_object();
	json_object_set_number(json_object(obb), "sizex", ((ComponentAABB*)getParent()->getComponent(C_AABB))->getOBB()->Size().x);
	json_object_set_number(json_object(obb), "sizey", ((ComponentAABB*)getParent()->getComponent(C_AABB))->getOBB()->Size().y);
	json_object_set_number(json_object(obb), "sizez", ((ComponentAABB*)getParent()->getComponent(C_AABB))->getOBB()->Size().z);

	json_object_set_value(config, "obb", obb);


}

ComponentColliderCube::~ComponentColliderCube()
{

	App->physics->DeleteBody(body);

};
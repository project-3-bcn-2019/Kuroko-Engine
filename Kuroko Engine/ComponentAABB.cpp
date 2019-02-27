#include "ComponentAABB.h"
#include "ComponentMesh.h"
#include "GameObject.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "Transform.h"
#include "ModuleCamera3D.h"
#include "Camera.h"

#include "glew-2.1.0\include\GL\glew.h"
#include "ComponentTransform.h"


ComponentAABB::ComponentAABB(GameObject* parent) : Component(parent, C_AABB)
{
	aabb = new AABB();
	obb = new OBB();

	transform = (ComponentTransform*)getParent()->getComponent(TRANSFORM);
}

ComponentAABB::~ComponentAABB()
{
	if (aabb) delete aabb;
	if (obb) delete obb;
}

void ComponentAABB::Reload()
{
	std::list<Component*> meshes;
	getParent()->getComponents(MESH, meshes);

	float3 min_point = float3::inf;
	float3 max_point = -float3::inf;

	for (auto it = meshes.begin(); it != meshes.end(); it++)
	{
		Mesh* mesh = ((ComponentMesh*)(*it))->getMesh();
		if (!mesh)
			continue;

		if (min_point.x > (mesh->getCentroid() - mesh->getHalfSize()).x) min_point.x = (mesh->getCentroid() - mesh->getHalfSize()).x;
		if (min_point.y > (mesh->getCentroid() - mesh->getHalfSize()).y) min_point.y = (mesh->getCentroid() - mesh->getHalfSize()).y;
		if (min_point.z > (mesh->getCentroid() - mesh->getHalfSize()).z) min_point.z = (mesh->getCentroid() - mesh->getHalfSize()).z;

		if (max_point.x < (mesh->getCentroid() + mesh->getHalfSize()).x) max_point.x = (mesh->getCentroid() + mesh->getHalfSize()).x;
		if (max_point.y < (mesh->getCentroid() + mesh->getHalfSize()).y) max_point.y = (mesh->getCentroid() + mesh->getHalfSize()).y;
		if (max_point.z < (mesh->getCentroid() + mesh->getHalfSize()).z) max_point.z = (mesh->getCentroid() + mesh->getHalfSize()).z;

	}

	if (min_point.IsFinite() && max_point.IsFinite())
	{
		getParent()->own_centroid = (min_point + max_point) / 2;
		getParent()->own_half_size = max_point - getParent()->own_centroid;
	}
	else
		getParent()->own_centroid = getParent()->own_half_size = float3::zero;

	*aabb = obb->MinimalEnclosingAABB();
}

bool ComponentAABB::Update(float dt)
{
	if (isActive())
	{
		if (parent->getComponent(MESH) != nullptr)
		{
			obb->pos = transform->global->getPosition();
			obb->r = { getParent()->own_half_size.x * transform->global->getScale().x, getParent()->own_half_size.y * transform->global->getScale().y, getParent()->own_half_size.z * transform->global->getScale().z };

			obb->axis[0] = transform->global->Right();
			obb->axis[1] = transform->global->Up();
			obb->axis[2] = transform->global->Forward();
		
			std::list<GameObject*> children;
			getParent()->getChildren(children);

			for (auto it = children.begin(); it != children.end(); it++)
			{
				OBB* child_obb = ((ComponentAABB*)(*it)->getComponent(C_AABB))->obb;
				for (int i = 0; i < 8; i++)
				{
					float3 test = child_obb->CornerPoint(i);
					obb->Enclose(child_obb->CornerPoint(i));
				}
			}

			getParent()->centroid = obb->pos;
			getParent()->half_size = obb->r.Abs();

			*aabb = obb->MinimalEnclosingAABB();
		}
		else
		{
			
			//float4x4 transform_m = float4x4::identity;

			//transform_m = float4x4::FromTRS(transform->global->getPosition(), transform->global->getRotation(), transform->global->getScale());

			//aabb->TransformAsAABB(transform->global->CalculateMatrix());
			
			//float3 min = aabb->minPoint;
			//float3 max = aabb->maxPoint;

			//aabb->minPoint = transform->global->getPosition();
			//aabb->minPoint.x -= (max.x - min.x) / 2;
			//aabb->minPoint.y -= (max.y - min.y) / 2;
			//aabb->minPoint.z -= (max.z - min.z) / 2;


			//aabb->maxPoint = transform->global->getPosition();
			//aabb->maxPoint.x += (max.x - min.x) / 2;
			//aabb->maxPoint.y += (max.y - min.y) / 2;
			//aabb->maxPoint.z += (max.z - min.z) / 2;

			//aabb-> = transform->global->getScale();

			//aabb->maxPoint = transform->global->getPosition() + ((aabb->maxPoint - aabb->minPoint) / 2);

			
		}

	}
	return true;
}

void ComponentAABB::Draw() const
{
	if (draw_aabb)	DrawAABB();
	if (draw_obb && App->camera->current_camera->frustumCull(*getOBB()))	DrawOBB();
}

void ComponentAABB::DrawAABB() const
{
	glLineWidth(1.5f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);

	for (int i = 0; i < 12; i++)
	{
		glVertex3f(aabb->Edge(i).a.x, aabb->Edge(i).a.y, aabb->Edge(i).a.z);		
		glVertex3f(aabb->Edge(i).b.x, aabb->Edge(i).b.y, aabb->Edge(i).b.z);
	}

	glEnd();
	glColor3f(1.0f, 1.0f, 1.0f);
	glLineWidth(1.0f);
}

void ComponentAABB::DrawOBB() const
{
	glLineWidth(1.5f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_LINES);

	for (int i = 0; i < 12; i++)
	{
		glVertex3f(obb->Edge(i).a.x, obb->Edge(i).a.y, obb->Edge(i).a.z);
		glVertex3f(obb->Edge(i).b.x, obb->Edge(i).b.y, obb->Edge(i).b.z);
	}

	
	glEnd();
	glColor3f(1.0f, 1.0f, 1.0f);

	glLineWidth(1.0f);
}

void ComponentAABB::Save(JSON_Object* config) {

	json_object_set_string(config, "type", "AABB");

}
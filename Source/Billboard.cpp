#include "Billboard.h"
#include "Component.h"
#include "Material.h"
#include "Camera.h"
#include "Transform.h"
#include "Mesh.h"
#include "GameObject.h"
#include "Application.h"
#include "ComponentAABB.h"
#include "ModuleScene.h"
#include "ModuleCamera3D.h"
#include "ResourceTexture.h"
#include "ModuleResourcesManager.h"
#include "ComponentAABB.h"
#include "ComponentBillboard.h"
#include "ComponentTransform.h"
#include "ComponentParticleEmitter.h"

#include "Parson\parson.h"
#include "glew-2.1.0\include\GL\glew.h"
#include <gl/GL.h>
#include <gl/GLU.h>


Billboard::Billboard(Component* p_component, Mesh* _mesh, Material* mat)
{
	parent_component = p_component;
	mesh = _mesh;
	camera = App->camera->current_camera;

	if (mat)		material = mat;
	else			material = new Material();

	if (parent_component->getType() == BILLBOARD)
		transform = new Transform(*((ComponentBillboard*)parent_component)->transform->global);
	else if(parent_component->getType() == PARTICLE_EMITTER)
		transform = new Transform(*((ComponentParticleEmitter*)parent_component)->transform->global);
}

Billboard::~Billboard()
{
	if(transform) delete transform;
	if (material) delete material;
}


Billboard::Billboard(Component* p_component, Mesh* _mesh, JSON_Object* deff)
{
	parent_component = p_component;
	mesh = _mesh;
	camera = App->camera->current_camera;

	wireframe = json_object_dotget_boolean(deff, "Wireframe");
	useColor = json_object_dotget_boolean(deff, "Use Color");

	color = parent_component->LoadColor(deff, "Color");

	alignment = (Alignment)((int)json_object_dotget_number(deff, "Alignment"));

	Material* material = new Material();
	// ASSIGNING RESOURCE

	const char* diffuse_path = json_object_dotget_string(deff, "material.diffuse");
	uint diffuse_resource = 0;
	if (diffuse_path) { // Means that is being loaded from a scene
		if (strcmp(diffuse_path, "missing reference") != 0)
			diffuse_resource = App->resources->getResourceUuid(diffuse_path);
	}
	else // Means it is being loaded from a 3dObject binary
		diffuse_resource = json_object_dotget_number(deff, "material.diffuse_resource_uuid");


	if (diffuse_resource != 0) {
		App->resources->assignResource(diffuse_resource);
		material->setTextureResource(DIFFUSE, diffuse_resource);
	}


	if (parent_component->getType() == BILLBOARD)
		transform = new Transform(*((ComponentBillboard*)parent_component)->transform->global);
	else if (parent_component->getType() == PARTICLE_EMITTER)
		transform = new Transform(*((ComponentParticleEmitter*)parent_component)->transform->global);
}

void Billboard::Save(JSON_Object* json)
{
	json_object_dotset_boolean(json, "Wireframe", wireframe);
	json_object_dotset_boolean(json, "Use Color", useColor);

	parent_component->SaveColor(json, "Color", color);

	json_object_dotset_number(json, "Alignment", alignment);

	if (getMaterial()) {  //If it has a material and a diffuse texture
		ResourceTexture* res_diff = (ResourceTexture*)App->resources->getResource(getMaterial()->getTextureResource(DIFFUSE));
		if (res_diff)
			json_object_dotset_string(json, "material.diffuse", res_diff->asset.c_str());
		else
			json_object_dotset_string(json, "material.diffuse", "missing_reference");
	}

}

void Billboard::Draw() const
{
	OBB obb;
	obb.r = float3(1.0,1.0, 0.01f); 
	obb.pos = transform->getPosition();
	   
	obb.axis[0] = transform->Right();
	obb.axis[1] = transform->Up();
	obb.axis[2] = transform->Forward();

	if (camera->frustumCull(obb))
	{
		float4x4 view_mat = float4x4::identity;

		if (transform)
		{
			GLfloat matrix[16];
			glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
			view_mat.Set((float*)matrix);

			glMatrixMode(GL_MODELVIEW_MATRIX);
			glLoadMatrixf((GLfloat*)(transform->getMatrix().Transposed() * view_mat).v);
		}


		if (wireframe || App->scene->global_wireframe)	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else											glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		mesh->Draw(material);

		if (transform)
			glLoadMatrixf((GLfloat*)view_mat.v);
	}

}


void Billboard::FaceCamera()
{

	if (camera != App->camera->current_camera)
		camera = App->camera->current_camera;

	switch (alignment)
	{
	case SCREEN_ALIGN:	ScreenAlign();	break;
	case WORLD_ALIGN:	WorldAlign();	break;
	case AXIAL_ALIGN:	AxialAlign();	break;
	default:
		break;
	}

	transform->CalculateMatrix();
}


void Billboard::WorldAlign()
{
	float3 normal = (camera->getFrustum()->pos - transform->getPosition()).Normalized();
	float3 up = camera->getFrustum()->up;
	float3 right = normal.Cross(up);
	float3 upF = normal.Cross(right);

	float3x3 mat = float3x3::identity;
	mat.Set(right.x, right.y, right.z, upF.x, upF.y, upF.z, normal.x, normal.y, normal.z);

	transform->setRotation(mat.Inverted().ToQuat());
}

void Billboard::ScreenAlign()
{
	float3 normal = (camera->getFrustum()->pos - transform->getPosition()).Normalized();
	float3 up = camera->getFrustum()->up;
	float3 right = normal.Cross(up);

	float3x3 mat = float3x3::identity;
	mat.Set(-right.x, -right.y, -right.z, up.x, up.y, up.z, normal.x, normal.y, normal.z);

	transform->setRotation(mat.Inverted().ToQuat());
}

void Billboard::AxialAlign()
{
	float3 normal = (camera->getFrustum()->pos - transform->getPosition()).Normalized();
	float3 up;
	up.Set(0.0f, 1.0f, 0.0f);
	float3 right = normal.Cross(up);

	float3x3 mat = float3x3::identity;
	mat.Set(right.x, right.y, right.z, 0.0f, 0.0f, 0.0f, normal.x, normal.y, normal.z);

	transform->setRotation(mat.Inverted().ToQuat());
	transform->getMatrix().Set(1, 2, 0.0f);
	transform->getMatrix().Set(2, 2, 0.0f);
	transform->getMatrix().Set(3, 2, 0.0f);
}

void Billboard::UpdateFromParticle(const Particle& particle)
{
	UpdateTransform(particle.position, float3(particle.size));
	color = particle.color;

	FaceCamera();

	if (useColor)			mesh->tint_color = color;
	else					mesh->tint_color = White;

	Draw();
}


void Billboard::UpdateTransform(float3 pos, float3 scl)
{
	transform->setPosition(pos);
	transform->setScale(float3(scl));
}

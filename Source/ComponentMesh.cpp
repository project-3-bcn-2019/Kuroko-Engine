#include "ComponentMesh.h"
#include "ComponentTransform.h"
#include "Transform.h"
#include "GameObject.h"
#include "ModuleScene.h"
#include "Application.h"
#include "ComponentAABB.h"
#include "ComponentBone.h"
#include "Material.h"
#include "ModuleImporter.h"
#include "FileSystem.h"
#include "glew-2.1.0\include\GL\glew.h"
#include "ModuleCamera3D.h"
#include "Camera.h"
#include "Applog.h"
#include "ModuleResourcesManager.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "ResourceBone.h"

ComponentMesh::ComponentMesh(JSON_Object * deff, GameObject* parent): Component(parent, MESH) {
	std::string path;

	// Load mesh from own file format
	
	//std::string mesh_name = json_object_get_string(deff, "mesh_name"); // Mesh name not used for now
	primitive_type = primitiveString2PrimitiveType(json_object_get_string(deff, "primitive_type"));

													 
	if(primitive_type == Primitive_None){			// TODO: Store the color of the meshes
		// ASSIGNING RESOURCE
		const char* parent3dobject = json_object_get_string(deff, "Parent3dObject");
		if (App->is_game && !App->debug_game)
		{
			setMeshResourceId(App->resources->getResourceUuid(json_object_get_string(deff, "mesh_name"), R_MESH));		
		}
		else if (parent3dobject) // Means that is being loaded from a scene
			mesh_resource_uuid = App->resources->getMeshResourceUuid(parent3dobject, json_object_get_string(deff, "mesh_name"));
		else // Means it is being loaded from a 3dObject binary
			mesh_resource_uuid = json_object_get_number(deff, "mesh_resource_uuid");

		App->resources->assignResource(mesh_resource_uuid);
	}

	JSON_Array* bones = json_object_get_array(deff, "bones");
	if (bones != nullptr) //There are stored bones
	{
		for (int i = 0; i < json_array_get_count(bones); ++i)
		{
			JSON_Object* bone = json_array_get_object(bones, i);
			bones_names.push_back(json_object_get_string(bone, "bone_name"));
		}
	}

	mat = new Material();
	// ASSIGNING RESOURCE

	const char* diffuse_path = json_object_dotget_string(deff, "material.diffuse");
	uint diffuse_resource = 0;
	if (diffuse_path) { // Means that is being loaded from a scene
		if(strcmp(diffuse_path, "missing reference") != 0)
			diffuse_resource = App->resources->getResourceUuid(diffuse_path);
	}
	else // Means it is being loaded from a 3dObject binary
		diffuse_resource = json_object_dotget_number(deff, "material.diffuse_resource_uuid");


	if(diffuse_resource != 0){
		App->resources->assignResource(diffuse_resource);
		mat->setTextureResource(DIFFUSE, diffuse_resource);
	}
}

ComponentMesh::~ComponentMesh() {
	// Deassign all the components that the element had if it is deleted
	if(primitive_type == Primitive_None)
		App->resources->deasignResource(mesh_resource_uuid);
	delete mat;
}

void ComponentMesh::Draw() const
{
	if (Mesh* mesh_from_resource = getMeshFromResource())
	{
		OBB* obb = ((ComponentAABB*)getParent()->getComponent(C_AABB))->getOBB();

		if (App->camera->current_camera->frustumCull(*obb))
		{
			ComponentTransform* transform = nullptr;
			float4x4 view_mat = float4x4::identity;


			if (transform = (ComponentTransform*)getParent()->getComponent(TRANSFORM))
			{
				GLfloat matrix[16];
				glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
				view_mat.Set((float*)matrix);

				glMatrixMode(GL_MODELVIEW_MATRIX);
				glLoadMatrixf((GLfloat*)(transform->global->getMatrix().Transposed() * view_mat).v);
			}

			if (draw_normals || App->scene->global_normals)
				mesh_from_resource->DrawNormals();

			if (wireframe || App->scene->global_wireframe)	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else											glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			Skining();
			mesh_from_resource->Draw(mat);
			//Descoment to use shader render
			/*ComponentAnimation* animation = nullptr;
			animation = (ComponentAnimation*)getParent()->getComponent(ANIMATION);
			mesh_from_resource->MaxDrawFunctionTest(mat, animation,*transform->global->getMatrix().Transposed().v);*/


			if (transform)
				glLoadMatrixf((GLfloat*)view_mat.v);
		}
	}
}

void ComponentMesh::DrawSelected() const
{
	if (Mesh* mesh_from_resource = getMeshFromResource())
	{
		OBB* obb = ((ComponentAABB*)getParent()->getComponent(C_AABB))->getOBB();

		if (App->camera->current_camera->frustumCull(*obb))
		{
			ComponentTransform* transform = nullptr;
			float4x4 view_mat = float4x4::identity;

			if (transform = (ComponentTransform*)getParent()->getComponent(TRANSFORM))
			{
				GLfloat matrix[16];
				glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
				view_mat.Set((float*)matrix);

				glMatrixMode(GL_MODELVIEW_MATRIX);
				glLoadMatrixf((GLfloat*)(transform->global->getMatrix().Transposed() * view_mat).v);
			}

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


			Mesh* mesh_from_resource = getMeshFromResource();


			mesh_from_resource->Draw(mat, true);
			//Descoment to use shader render
			/*ComponentAnimation* animation = nullptr;
			animation = (ComponentAnimation*)getParent()->getComponent(ANIMATION);
			mesh_from_resource->MaxDrawFunctionTest(nullptr,animation,*transform->global->getMatrix().Transposed().v, true);*/

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			if (transform)
				glLoadMatrixf((GLfloat*)view_mat.v);
		}
	}
}

bool ComponentMesh::Update(float dt)
{
	if (components_bones.size() == 0 && parent->getParent() != nullptr && bones_names.size() > 0)
	{
		setMeshResourceId(mesh_resource_uuid);
	}

	return true;
}

Mesh* ComponentMesh::getMesh() const {

	Mesh* ret = nullptr;
	ResourceMesh* mesh_resource = nullptr;
	if (primitive_type == Primitive_None) {
		mesh_resource = (ResourceMesh*)App->resources->getResource(mesh_resource_uuid);
		if(mesh_resource)
			ret = mesh_resource->mesh;
	}
	else {
		mesh_resource = (ResourceMesh*)App->resources->getPrimitiveMeshResource(primitive_type);
		ret = mesh_resource->mesh;
	}
	return ret;
}
void ComponentMesh::setMeshResourceId(uint _mesh_resource_uuid) {

	App->resources->deasignResource(mesh_resource_uuid);
	mesh_resource_uuid = _mesh_resource_uuid;
	App->resources->assignResource(mesh_resource_uuid);
	((ComponentAABB*)getParent()->getComponent(C_AABB))->Reload();

	components_bones.clear();
	for (int i = 0; i < bones_names.size(); ++i)
	{
		GameObject* GO = parent->getAbsoluteParent()->getChild(bones_names[i].c_str(), true);
		if (GO != nullptr)
		{
			Component* bone = GO->getComponent(BONE);
			if (bone != nullptr)
			{
				components_bones.push_back(bone->getUUID());
			}
		}
	}
}
PrimitiveTypes ComponentMesh::primitiveString2PrimitiveType(std::string primitive_type_string) {

	PrimitiveTypes ret = Primitive_None; // Just for security
	if (primitive_type_string == "CUBE")
		ret = Primitive_Cube;
	else if (primitive_type_string == "PLANE")
		ret = Primitive_Plane;
	else if (primitive_type_string == "SPHERE")
		ret = Primitive_Sphere;
	else if (primitive_type_string == "CYLINDER")
		ret = Primitive_Cylinder;
	
	return ret;
}

std::string ComponentMesh::PrimitiveType2primitiveString(PrimitiveTypes type) {
	std::string ret = "NONE";

	switch (type) {
	case Primitive_Cube:
		ret = "CUBE";
		break;
	case Primitive_Plane:
		ret = "PLANE";
		break;
	case Primitive_Sphere:
		ret = "SPHERE";
		break;
	case Primitive_Cylinder:
		ret = "CYLINDER";
		break;

	}
	return ret;
}

void ComponentMesh::Skining() const
{
	ResourceMesh* mesh = (ResourceMesh*)App->resources->getResource(mesh_resource_uuid);
	if (mesh != nullptr && components_bones.size() > 0 && parent->getParent() != nullptr && bones_names.size() > 0)
	{		
		float3* vertices = new float3[mesh->mesh->getNumVertices()];
		memset(vertices, 0, sizeof(float)*mesh->mesh->getNumVertices() * 3);

		bool hasBones = false;
		for (int i = 0; i < components_bones.size(); i++)
		{
			GameObject* absoluteParent = parent->getAbsoluteParent();
			ComponentBone* bone = (ComponentBone*)absoluteParent->getChildComponent(components_bones[i]);
			if (bone != nullptr)
			{
				ResourceBone* rBone = (ResourceBone*)App->resources->getResource(bone->getBoneResource());
				if (rBone != nullptr)
				{
					hasBones = true;
					//float4x4 boneTransform = ((ComponentTransform*)bone->getParent()->getComponent(TRANSFORM))->global->getMatrix()*rBone->Offset;
					float4x4 boneTransform = ((ComponentTransform*)parent->getComponent(TRANSFORM))->global->getMatrix().Inverted()*((ComponentTransform*)bone->getParent()->getComponent(TRANSFORM))->global->getMatrix()*rBone->Offset;

					for (int j = 0; j < rBone->numWeights; j++)
					{
						uint VertexIndex = rBone->weights[j].VertexID;


						if (VertexIndex >= mesh->mesh->getNumVertices())
							continue;
						float3 startingVertex(mesh->mesh->getVertices()[VertexIndex]);
						float3 movementWeight = boneTransform.TransformPos(mesh->mesh->getVertices()[VertexIndex] + mesh->mesh->getCentroid());

						/*vertices[VertexIndex].x += movementWeight.x*rBone->weights[j].weight;
						vertices[VertexIndex].y += movementWeight.y*rBone->weights[j].weight;
						vertices[VertexIndex].z += movementWeight.z*rBone->weights[j].weight;*/
						vertices[VertexIndex] += movementWeight * rBone->weights[j].weight;
					}
				}
			}
		}

		if (hasBones)
			mesh->mesh->setMorphedVertices(vertices);
		else
			mesh->mesh->setMorphedVertices(nullptr);

		mesh->mesh->updateVRAM();
	}
}

void ComponentMesh::Save(JSON_Object* config) {
	// Determine the type of the mesh
 	// Component has two strings, one for mesh name, and another for diffuse texture name
	json_object_set_string(config, "type", "mesh");

	if(mesh_resource_uuid != 0){
		//json_object_set_number(config, "mesh_resource_uuid", mesh_resource_uuid);
		ResourceMesh* res_mesh = (ResourceMesh*)App->resources->getResource(mesh_resource_uuid);
		if(res_mesh){
			json_object_set_string(config, "mesh_name", res_mesh->asset.c_str());
			json_object_set_string(config, "Parent3dObject", res_mesh->Parent3dObject.c_str());
		}
		else {
			json_object_set_string(config, "mesh_name", "missing reference");
			json_object_set_string(config, "Parent3dObject", "missing reference");
		}
	}
	json_object_set_string(config, "primitive_type", PrimitiveType2primitiveString(primitive_type).c_str());
	if (mat) {  //If it has a material and a diffuse texture
		ResourceTexture* res_diff = (ResourceTexture*)App->resources->getResource(mat->getTextureResource(DIFFUSE));
		if(res_diff)
			json_object_dotset_string(config, "material.diffuse",res_diff->asset.c_str());
		else
			json_object_dotset_string(config, "material.diffuse", "missing_reference");
	}
	if (components_bones.size() > 0) //If it has any bone
	{
		JSON_Value* bones = json_value_init_array();
		for (int i = 0; i < components_bones.size(); ++i)
		{
			JSON_Value* bone = json_value_init_object();
			json_object_set_string(json_object(bone), "bone_name", bones_names[i].c_str());
			json_array_append_value(json_array(bones), bone);
		}
		json_object_set_value(config, "bones", bones);
	}
}

Mesh * ComponentMesh::getMeshFromResource() const
{
	ResourceMesh* mesh_resource = nullptr;
	Mesh* mesh = nullptr;

	if (primitive_type == Primitive_None) {
		mesh_resource = (ResourceMesh*)App->resources->getResource(mesh_resource_uuid);
		if (mesh_resource)
			mesh = mesh_resource->mesh;
	}
	else {
		mesh_resource = (ResourceMesh*)App->resources->getPrimitiveMeshResource(primitive_type);
		mesh = mesh_resource->mesh;
	}
	return mesh;
}

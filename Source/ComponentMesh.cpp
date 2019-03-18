#include "ComponentMesh.h"
#include "ComponentTransform.h"
#include "Transform.h"
#include "GameObject.h"
#include "ModuleScene.h"
#include "ModuleShaders.h"
#include "Application.h"
#include "ComponentAABB.h"
#include "ComponentBone.h"
#include "Material.h"
#include "ModuleImporter.h"
#include "ModuleUI.h"
#include "FileSystem.h"
#include "glew-2.1.0\include\GL\glew.h"
#include "ModuleCamera3D.h"
#include "Camera.h"
#include "Applog.h"
#include "ModuleResourcesManager.h"
#include "ModuleRenderer3D.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "ResourceBone.h"
#include "ResourceShader.h"

std::string openFileWID(bool isfile = false);

ComponentMesh::ComponentMesh(JSON_Object * deff, GameObject* parent): Component(parent, MESH) {
	is_active = json_object_get_boolean(deff, "active");
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
		if (strcmp(diffuse_path, "missing reference") != 0)
		{
			if (!App->is_game || App->debug_game)
				diffuse_resource = App->resources->getResourceUuid(diffuse_path);
			else
			{
				std::string d_path = diffuse_path;
				App->fs.getFileNameFromPath(d_path);
				diffuse_resource = App->resources->getTextureResourceUuid(d_path.c_str());
			}
		}
	}
	else // Means it is being loaded from a 3dObject binary
		diffuse_resource = json_object_dotget_number(deff, "material.diffuse_resource_uuid");

	const char* vertex_path = json_object_dotget_string(deff, "vertex_shader");
	uint vertex_resource = 0;
	if (vertex_path) { // Means that is being loaded from a scene	
		if (!App->is_game || App->debug_game)
			vertex_resource = App->resources->getResourceUuid(vertex_path);
		else
		{
			std::string s_path = vertex_path;
			App->fs.getFileNameFromPath(s_path);
			vertex_resource = App->resources->getShaderResourceUuid(s_path.c_str());
		}
	}
	//else // Means it is being loaded from a 3dObject binary
	//	vertex_resource = json_object_dotget_number(deff, "material.diffuse_resource_uuid");

	const char* fragment_path = json_object_dotget_string(deff, "fragment_shader");
	uint fragment_resource = 0;
	if (fragment_path) { // Means that is being loaded from a scene	
		if (!App->is_game || App->debug_game)
			fragment_resource = App->resources->getResourceUuid(fragment_path);
		else
		{
			std::string d_path = fragment_path;
			App->fs.getFileNameFromPath(d_path);
			fragment_resource = App->resources->getShaderResourceUuid(d_path.c_str());
		}
	}

	if(diffuse_resource != 0){
		App->resources->assignResource(diffuse_resource);
		mat->setTextureResource(DIFFUSE, diffuse_resource);
	}

	if (vertex_resource != 0 && fragment_resource != 0)
	{
		App->resources->assignResource(vertex_resource);
		App->resources->assignResource(fragment_resource);

		//first we look for an existent program
		uint ShaderID = App->shaders->GetShaderProgramByResources(vertex_resource, fragment_resource);
		if (ShaderID != 0)
		{
			mat->setShaderProgram(ShaderID);
		}
		else //if we can't find a program we create a new one
		{
			ShaderProgram* shader_program = new ShaderProgram();
			shader_program->shaderUUIDS.push_back(vertex_resource);
			shader_program->shaderUUIDS.push_back(fragment_resource);

			if (App->shaders->CompileProgramFromResources(shader_program))
			{
				App->shaders->shader_programs.push_back(shader_program);
			}
			else
			{
				//if there is any compiling error we will use the default shader that will be always loaded
				RELEASE(shader_program)
			}
		}
	}
}

ComponentMesh::~ComponentMesh() {
	// Deassign all the components that the element had if it is deleted
	if(primitive_type == Primitive_None)
		App->resources->deasignResource(mesh_resource_uuid);
	delete mat;
}

void ComponentMesh::Draw()
{
	if (mat)
	{
		if (mat->translucent)
		{
			App->renderer3D->translucentMeshes.push(this);
			return;
		}
	}
	App->renderer3D->opaqueMeshes.push_back(this);
}

void ComponentMesh::DrawSelected()
{
	App->renderer3D->selected_meshes_to_draw.push_back(this);
}

void ComponentMesh::Render() const
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

void ComponentMesh::RenderSelected() const
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

bool ComponentMesh::DrawInspector(int id)
{
	std::string tag;
	tag = "Mesh##" + std::to_string(id);
	if (ImGui::CollapsingHeader(tag.c_str()))
	{
		static bool wireframe_enabled;
		static bool mesh_active;
		static bool draw_normals;

		wireframe_enabled = getWireframe();
		draw_normals = getDrawNormals();
		mesh_active = isActive();

		if (ImGui::Checkbox("Active## mesh_active", &mesh_active))
		{
			setActive(mesh_active);
			App->scene->AskAutoSaveScene();
		}
		if (mesh_active)
		{
			ResourceMesh* R_mesh = (ResourceMesh*)App->resources->getResource(getMeshResource());
			ImGui::Text("Resource: %s", (R_mesh != nullptr) ? R_mesh->asset.c_str() : "None");

			if (ImGui::Checkbox("Wireframe", &wireframe_enabled))
				setWireframe(wireframe_enabled);
			ImGui::SameLine();
			if (ImGui::Checkbox("Draw normals", &draw_normals))
				setDrawNormals(draw_normals);

			if (!getMesh()) {
				static bool add_mesh_menu = false;
				if (ImGui::Button("Add mesh")) {
					add_mesh_menu = true;
				}

				if (add_mesh_menu) {

					std::list<resource_deff> mesh_res;
					App->resources->getMeshResourceList(mesh_res);

					ImGui::Begin("Mesh selector", &add_mesh_menu);
					for (auto it = mesh_res.begin(); it != mesh_res.end(); it++) {
						resource_deff mesh_deff = (*it);
						if (ImGui::MenuItem(mesh_deff.asset.c_str())) {
							App->resources->deasignResource(getMeshResource());
							App->resources->assignResource(mesh_deff.uuid);
							setMeshResourceId(mesh_deff.uuid);
							add_mesh_menu = false;
							break;
						}
					}

					ImGui::End();
				}
			}

			if (Mesh* mesh = getMesh())
			{
				if (ImGui::TreeNode("Mesh Options"))
				{
					uint vert_num, poly_count;
					bool has_normals, has_colors, has_texcoords;
					if (ImGui::Button("Remove mesh")) {
						App->resources->deasignResource(getMeshResource());
						setMeshResourceId(0);
					}
					mesh->getData(vert_num, poly_count, has_normals, has_colors, has_texcoords);
					ImGui::Text("vertices: %d, poly count: %d, ", vert_num, poly_count);
					ImGui::Text(has_normals ? "normals: Yes," : "normals: No,");
					ImGui::Text(has_colors ? "colors: Yes," : "colors: No,");
					ImGui::Text(has_texcoords ? "tex coords: Yes" : "tex coords: No");

					ImGui::TreePop();
				}
			}


			if (ImGui::TreeNode("Material"))
			{
				if (Material* material = getMaterial())
				{
					ImGui::Checkbox("translucent", &material->translucent);

					static int preview_size = 128;
					ImGui::Text("Id: %d", material->getId());
					ImGui::SameLine();
					/*if (ImGui::Button("remove material"))
					{
						delete c_mesh->getMaterial();
						c_mesh->setMaterial(nullptr);
						ImGui::TreePop();
						return true;
					}*/

					ImGui::Text("Preview size");
					ImGui::SameLine();
					if (ImGui::Button("64")) preview_size = 64;
					ImGui::SameLine();
					if (ImGui::Button("128")) preview_size = 128;
					ImGui::SameLine();
					if (ImGui::Button("256")) preview_size = 256;

					if (ImGui::TreeNode("diffuse"))
					{
						Texture* texture = nullptr;
						if (ResourceTexture* tex_res = (ResourceTexture*)App->resources->getResource(material->getTextureResource(DIFFUSE)))
							texture = tex_res->texture;


						ImGui::Image(texture ? (void*)texture->getGLid() : (void*)App->gui->ui_textures[NO_TEXTURE]->getGLid(), ImVec2(preview_size, preview_size));
						ImGui::SameLine();

						int w = 0; int h = 0;
						if (texture)
							texture->getSize(w, h);

						ImGui::Text("texture data: \n x: %d\n y: %d", w, h);

						//if (ImGui::Button("Load checkered##Dif: Load checkered"))
						//	material->setCheckeredTexture(DIFFUSE);
						//ImGui::SameLine()
						if (ImGui::Button("Load(from asset folder)##Dif: Load"))
						{
							std::string texture_path = openFileWID();
							uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
							if (new_resource != 0) {
								App->resources->assignResource(new_resource);
								App->resources->deasignResource(material->getTextureResource(DIFFUSE));
								material->setTextureResource(DIFFUSE, new_resource);
							}
						}
						ImGui::TreePop();
					}

					if (ImGui::TreeNode("shader program"))
					{
						if (material->getShaderProgram())
						{
							ImGui::Text("Vertex:");
							ImGui::SameLine();

							ResourceShader* vertex_shader = (ResourceShader*)App->resources->getResource(material->getShaderProgram()->shaderUUIDS[0]);

							std::string vertex_name;
							if (vertex_shader)
							{
								vertex_name = vertex_shader->asset;
								App->fs.getFileNameFromPath(vertex_name);
							}
							else
							{
								vertex_name = "NONE";
							}

							ImGui::PushItemWidth(200.0f);
							ImGui::PushID("Vertex Shader");

							if (ImGui::BeginCombo("", vertex_name.c_str()))
							{
								std::list<resource_deff> vertex_Sahders;
								App->resources->getShaderResourceList(vertex_Sahders);

								for (std::list<resource_deff>::iterator v_it = vertex_Sahders.begin(); v_it != vertex_Sahders.end(); ++v_it)
								{
									Shader* shader = nullptr;
									if (shader=((ResourceShader*)App->resources->getResource((*v_it).uuid))->shaderObject)
									{
										bool selected = false;

										if (shader->type == VERTEX)
										{
											if (ImGui::Selectable(shader->name.c_str(), &selected))
											{
												/*change the shader and recompile the shader program*/
											}
										}
									}			
								}
								ImGui::EndCombo();
							}
							ImGui::PopID();
							ImGui::PopItemWidth();

							ImGui::Text("Fragment:");
							ImGui::SameLine();
							ResourceShader* fragment_shader = (ResourceShader*)App->resources->getResource(material->getShaderProgram()->shaderUUIDS[1]);

							std::string fragment_name;
							if (fragment_shader)
							{
								fragment_name = fragment_shader->asset;
								App->fs.getFileNameFromPath(fragment_name);
							}
							else
							{
								fragment_name = "NONE";
							}

							ImGui::PushItemWidth(200.0f);
							ImGui::PushID("Fragment Shader");

							if (ImGui::BeginCombo("", fragment_name.c_str()))
							{
								std::list<resource_deff> fragment_Sahders;
								App->resources->getShaderResourceList(fragment_Sahders);

								for (std::list<resource_deff>::iterator v_it = fragment_Sahders.begin(); v_it != fragment_Sahders.end(); ++v_it)
								{
									Shader* shader = nullptr;

									if (shader = ((ResourceShader*)App->resources->getResource((*v_it).uuid))->shaderObject)
									{
										bool selected = false;
										if (shader->type == FRAGMENT)
										{
											if (ImGui::Selectable(shader->name.c_str(), &selected))
											{
												/*change the shader and recompile the shader program*/
											}
										}

									}
								}
								ImGui::EndCombo();
							}
							ImGui::PopID();
							ImGui::PopItemWidth();
						}
						ImGui::TreePop();
					}

					if (ImGui::TreeNode("ambient (feature not avaliable yet)"))
					{
						//ImGui::Image(material->getTexture(AMBIENT) ? (void*)material->getTexture(AMBIENT)->getGLid() : (void*)ui_textures[NO_TEXTURE]->getGLid(), ImVec2(preview_size, preview_size));

						//if (ImGui::Button("Load checkered##Amb: Load checkered"))
						//	material->setCheckeredTexture(AMBIENT);
						//ImGui::SameLine();
						//if (ImGui::Button("Load##Amb: Load"))
						//{
						//	std::string texture_path = openFileWID();
						//	if (Texture* tex = (Texture*)App->importer->Import(texture_path.c_str(), I_TEXTURE))
						//		c_mesh->getMaterial()->setTexture(AMBIENT, tex);
						//}
						ImGui::TreePop();
					}

					if (ImGui::TreeNode("normals (feature not avaliable yet)"))
					{
						//ImGui::Image(material->getTexture(NORMALS) ? (void*)material->getTexture(NORMALS)->getGLid() : (void*)ui_textures[NO_TEXTURE]->getGLid(), ImVec2(preview_size, preview_size));

						//if (ImGui::Button("Load checkered##Nor: Load checkered"))
						//	material->setCheckeredTexture(NORMALS);
						//ImGui::SameLine();
						//if (ImGui::Button("Load##Nor: Load"))
						//{
						//	std::string texture_path = openFileWID();
						//	if (Texture* tex = (Texture*)App->importer->Import(texture_path.c_str(), I_TEXTURE))
						//		c_mesh->getMaterial()->setTexture(NORMALS, tex);
						//}
						ImGui::TreePop();
					}

					if (ImGui::TreeNode("lightmap (feature not avaliable yet)"))
					{
						//ImGui::Image(material->getTexture(LIGHTMAP) ? (void*)material->getTexture(LIGHTMAP)->getGLid() : (void*)ui_textures[NO_TEXTURE]->getGLid(), ImVec2(preview_size, preview_size));

						//if (ImGui::Button("Load checkered##Lgm: Load checkered"))
						//	material->setCheckeredTexture(LIGHTMAP);
						//ImGui::SameLine();
						//if (ImGui::Button("Load##Lgm: Load"))
						//{
						//	std::string texture_path = openFileWID();
						//	if (Texture* tex = (Texture*)App->importer->Import(texture_path.c_str(), I_TEXTURE))
						//		c_mesh->getMaterial()->setTexture(LIGHTMAP, tex);
						//}
						ImGui::TreePop();
					}
				}
				else
				{
					ImGui::TextWrapped("No material assigned!");

					if (getMesh())
					{
						static bool draw_colorpicker = false;
						static Color reference_color = getMesh()->tint_color;
						static GameObject* last_selected = getParent();

						std::string label = getParent()->getName() + " color picker";

						if (last_selected != getParent())
							reference_color = getMesh()->tint_color;

						ImGui::SameLine();
						if (ImGui::ColorButton((label + "button").c_str(), ImVec4(getMesh()->tint_color.r, getMesh()->tint_color.g, getMesh()->tint_color.b, getMesh()->tint_color.a)))
							draw_colorpicker = !draw_colorpicker;

						if (draw_colorpicker)
							App->gui->DrawColorPickerWindow(label.c_str(), (Color*)&getMesh()->tint_color, &draw_colorpicker, (Color*)&reference_color);
						else
							reference_color = getMesh()->tint_color;

						last_selected = getParent();
					}

					if (ImGui::Button("Add material"))
					{
						Material* mat = new Material();
						setMaterial(mat);
					}

				}
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Connected Bones"))
			{
				ImGui::Text("Num Bones: %d", components_bones.size());
				ImGui::TreePop();
			}

		}
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.f, 0.f, 0.f, 1.f)); ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.f, 0.2f, 0.f, 1.f));
		if (ImGui::Button("Remove##Remove mesh")) {
			ImGui::PopStyleColor(); ImGui::PopStyleColor();
			return false;
		}
		ImGui::PopStyleColor(); ImGui::PopStyleColor();
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
	json_object_set_boolean(config, "active", is_active);

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

		if (mat->getShaderProgramID() != 0)
		{
			ShaderProgram* shader = mat->getShaderProgram();
			if (shader)
			{
				ResourceShader* vertex = (ResourceShader*)App->resources->getResource(shader->shaderUUIDS[0]);
				if(vertex)
					json_object_dotset_string(config, "vertex_shader", vertex->asset.c_str());

				ResourceShader* fragment = (ResourceShader*)App->resources->getResource(shader->shaderUUIDS[1]);
				if(fragment)
					json_object_dotset_string(config, "fragment_shader", fragment->asset.c_str());
			}
		}
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

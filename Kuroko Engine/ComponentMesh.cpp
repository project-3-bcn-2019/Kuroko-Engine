#include "ComponentMesh.h"
#include "ComponentTransform.h"
#include "Transform.h"
#include "GameObject.h"
#include "ModuleScene.h"
#include "Application.h"
#include "Material.h"
#include "glew-2.1.0\include\GL\glew.h"

void ComponentMesh::Draw() const
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
		mesh->DrawNormals();

	if (wireframe || App->scene->global_wireframe)	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else											glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	mesh->Draw(mat);

	if (transform)
		glLoadMatrixf((GLfloat*)view_mat.v);
}

void ComponentMesh::Save(JSON_Object & config) {
	// Determine the type of the mesh
	json_object_set_string(&config, "type", "mesh");
	json_object_set_string(&config, "mesh_name", mesh->getName());
	json_object_set_string(&config, "diffuse_name", mat->getTexture(DIFFUSE)->getName());
}

void ComponentMesh::Load(JSON_Object & config) {
}

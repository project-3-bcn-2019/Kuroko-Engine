#include "UI_PrimitivesTab.h"
#include "GameObject.h"
#include "ComponentMesh.h"

void UI_PrimitivesTab::UpdateContent()
{
	if (ImGui::Button("Add cube")) {
		GameObject* cube = new GameObject("Cube");
		cube->addComponent(C_AABB);
		cube->addComponent(new ComponentMesh(cube, Primitive_Cube));
	}
	ImGui::SameLine();
	if (ImGui::Button("Add plane")) {
		GameObject* plane = new GameObject("Plane");
		plane->addComponent(C_AABB);
		plane->addComponent(new ComponentMesh(plane, Primitive_Plane));
	}
	if (ImGui::Button("Add sphere")) {
		GameObject* sphere = new GameObject("Sphere");
		sphere->addComponent(C_AABB);
		sphere->addComponent(new ComponentMesh(sphere, Primitive_Sphere));
	}
	ImGui::SameLine();
	if (ImGui::Button("Add cylinder")) {
		GameObject* cylinder = new GameObject("Cylinder");
		cylinder->addComponent(C_AABB);
		cylinder->addComponent(new ComponentMesh(cylinder, Primitive_Cylinder));
	}

}

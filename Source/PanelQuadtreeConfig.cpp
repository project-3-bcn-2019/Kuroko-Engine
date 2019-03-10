#include "PanelQuadtreeConfig.h"
#include "Application.h"
#include "ModuleScene.h"
#include "Quadtree.h"

PanelQuadtreeConfig::PanelQuadtreeConfig(const char * name):Panel(name)
{
}

PanelQuadtreeConfig::~PanelQuadtreeConfig()
{
}

void PanelQuadtreeConfig::Draw()
{
	ImGui::Begin("Quadtree", &active);
	ImGui::Checkbox("Draw", &App->scene->draw_quadtree);
	ImGui::SameLine();
	if (ImGui::Button("Reload")) {
		App->scene->quadtree_reload = true;
	}
	static float3 size = float3(0, 0, 0);
	static float3 centre = float3(0, 0, 0);
	static int bucket_size = 1;
	static int max_depth = 8;
	static float size_arr[3] = { 0 };
	static float centre_arr[3] = { 0 };

	ImGui::InputFloat3("Centre", centre_arr);
	centre.x = centre_arr[0];
	centre.y = centre_arr[1];
	centre.z = centre_arr[2];

	ImGui::InputFloat3("Size", size_arr);
	size.x = size_arr[0];
	size.y = size_arr[1];
	size.z = size_arr[2];

	ImGui::InputInt("Bucket size", &bucket_size);
	ImGui::InputInt("Max depth", &max_depth);

	if (ImGui::Button("Create")) {
		AABB aabb;
		aabb.SetFromCenterAndSize(centre, size);
		App->scene->getQuadtree()->Create(aabb, bucket_size, max_depth);
		App->scene->quadtree_reload = true;
	}
	ImGui::Text("Quadtree ignored objects: %i", App->scene->quadtree_ignored_obj);
	ImGui::Text("Frustum checks against quadtree: %i", App->scene->quadtree_checks);
	// TODO: Be able to change bucket size, max depth and size.
	ImGui::End();
}

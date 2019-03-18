#include "PanelCameraMenu.h"
#include "Application.h"
#include "ModuleCamera3D.h"
#include "Camera.h"
#include "ComponentCamera.h"
#include "GameObject.h"

PanelCameraMenu::PanelCameraMenu(const char * name):Panel(name)
{
}

PanelCameraMenu::~PanelCameraMenu()
{
}

void PanelCameraMenu::Draw()
{
	ImGui::Begin("Camera Menu", &active);

	if (App->camera->override_editor_cam_culling)
	{
		ImGui::TextWrapped("Background camera frustum culling overriden by camera %s", App->camera->override_editor_cam_culling->getParent()->getParent()->getName().c_str());
		if (ImGui::Button("Stop overriding"))
			App->camera->override_editor_cam_culling = nullptr;
	}

	static bool hide_viewports = false;

	ImGui::Checkbox("Hide viewports", &hide_viewports);

	for (auto it = App->camera->game_cameras.begin(); it != App->camera->game_cameras.end(); it++)
	{
		if ((*it)->IsViewport() && hide_viewports)
			continue;

		std::string name;
		if ((*it)->getParent())
			name = (*it)->getParent()->getParent()->getName() + "Camera";
		else
		{
			if ((*it) == App->camera->editor_camera)
				name = "Free camera";
			else
				name = (*it)->getViewportDirString();

			if (App->camera->game_camera == (*it))
				name += "(Game camera)";
		}

		if (ImGui::TreeNode(name.c_str()))
		{
			if ((*it)->active)  ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Active");
			else				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Unactive");

			ImGui::SameLine();

			static bool is_overriding;
			is_overriding = ((*it) == App->camera->override_editor_cam_culling);
			if (ImGui::Checkbox("Override Frustum Culling", &is_overriding))
			{
				if (!is_overriding)  App->camera->override_editor_cam_culling = nullptr;
				else				 App->camera->override_editor_cam_culling = *it;
			}

			static bool game_camera;
			game_camera = (App->camera->game_camera == (*it));

			if (ImGui::Checkbox("Game Camera", &game_camera))
				App->camera->game_camera = game_camera ? (*it) : App->camera->editor_camera;
					

			ImGui::Checkbox("Draw camera view", &(*it)->draw_in_UI);

			ImGui::Checkbox("Draw frustum", &(*it)->draw_frustum);

			ImGui::Checkbox("Draw depth", &(*it)->draw_depth);

			if (ImGui::CollapsingHeader("Frustum"))
			{
				if ((*it)->getFrustum()->type == math::FrustumType::PerspectiveFrustum)
				{
					ImGui::Text("Current mode: Perspective");
					ImGui::SameLine();
					if (ImGui::Button("Set ortographic"))
					{
						(*it)->getFrustum()->type = math::FrustumType::OrthographicFrustum;
						(*it)->getFrustum()->orthographicHeight = (*it)->getFrustum()->orthographicWidth = INIT_ORT_SIZE;
					}

					static float ver_fov = RADTODEG * (*it)->getFrustum()->verticalFov;

					if (ImGui::SliderFloat("##Vertical FOV", &ver_fov, MIN_V_FOV, MAX_V_FOV, "Ver. FOV: %.0f"))
						(*it)->getFrustum()->verticalFov = DEGTORAD * ver_fov;
				}
				else
				{
					ImGui::Text("Current mode: Ortographic");
					ImGui::SameLine();

					if (ImGui::Button("Set perspective"))
					{
						(*it)->getFrustum()->type = math::FrustumType::PerspectiveFrustum;
						(*it)->getFrustum()->horizontalFov = DEGTORAD * INIT_HOR_FOV; (*it)->getFrustum()->verticalFov = DEGTORAD * INIT_VER_FOV;
					}

					ImGui::SliderFloat("##Ortographic Width", &(*it)->getFrustum()->orthographicWidth, 1.0f, 500.0f, "Ort. width: %.0f");
					ImGui::SliderFloat("##Ortographic Height", &(*it)->getFrustum()->orthographicHeight, 1.0f, 500.0f, "Ort. height: %.0f");

				}

				ImGui::SliderFloat("##Near Plane:", &(*it)->getFrustum()->nearPlaneDistance, 0.1f, (*it)->getFrustum()->farPlaneDistance, "Near plane: %.1f");
				ImGui::SliderFloat("##Far Plane:", &(*it)->getFrustum()->farPlaneDistance, (*it)->getFrustum()->nearPlaneDistance, 2500.0f, "Far plane: %.1f");

			}

			ImGui::TreePop();
		}
	}

	ImGui::End();
}

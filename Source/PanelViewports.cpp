#include "PanelViewports.h"
#include "Application.h"
#include "ModuleCamera3D.h"
#include "Camera.h"
#include "Material.h"

PanelViewports::PanelViewports(const char * name):Panel(name)
{
}

PanelViewports::~PanelViewports()
{
}

void PanelViewports::Draw()
{
	ImGui::Begin("Viewports", nullptr);

	if (ImGui::Button("Close##Close viewports"))
	{
		for (int i = 0; i < 6; i++)
			App->camera->viewports[i]->active = false;

		active = false;
	}

	for (int i = 0; i < 6; i++)
	{
		if (i != 0 && i != 3)
			ImGui::SameLine();

		FrameBuffer* fb = App->camera->viewports[i]->getFrameBuffer();
		ImGui::TextWrapped(App->camera->viewports[i]->getViewportDirString().c_str());
		ImGui::SameLine();

		if (ImGui::ImageButton((void*)(App->camera->viewports[i]->draw_depth ? fb->depth_tex->gl_id : fb->tex->gl_id), ImVec2(fb->size_x / 4, fb->size_y / 4), nullptr, ImVec2(0, 1), ImVec2(1, 0)))
		{
			/*for (int i = 0; i < 6; i++)
				App->camera->viewports[i]->active = false;

			App->camera->background_camera->active = false;
			App->camera->background_camera = App->camera->selected_camera = App->camera->viewports[i];
			App->camera->background_camera->active = true;
			active = false;*/
		}
	}
	ImGui::End();

}

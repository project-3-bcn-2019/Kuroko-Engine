#include "PanelConfiguration.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleCamera3D.h"
#include "Camera.h"
#include "ModuleWindow.h"
#include  "VRAM.h"
#include "ModuleResourcesManager.h"

#include "glew-2.1.0\include\GL\glew.h"

PanelConfiguration::PanelConfiguration(const char * name, bool active):Panel(name, active)
{
}

PanelConfiguration::~PanelConfiguration()
{
}

void PanelConfiguration::Draw()
{
	ImGui::Begin("Configuration", &active);

	if (ImGui::CollapsingHeader("Graphics"))
		DrawGraphicsLeaf();
	if (ImGui::CollapsingHeader("Window"))
		DrawWindowConfigLeaf();
	if (ImGui::CollapsingHeader("Hardware"))
		DrawHardwareLeaf();
	if (ImGui::CollapsingHeader("Application"))
		DrawApplicationLeaf();
	if (ImGui::CollapsingHeader("Editor preferences"))
		DrawEditorPreferencesLeaf();
	if (ImGui::CollapsingHeader("Resources"))
		DrawResourcesLeaf();

	if (ImGui::Button("Reset Camera"))
		App->camera->editor_camera->Reset();

	ImGui::End();
}

void PanelConfiguration::DrawGraphicsLeaf() const {
	//starting values

	static bool depth_test = glIsEnabled(GL_DEPTH_TEST);
	static bool face_culling = glIsEnabled(GL_CULL_FACE);
	static bool lighting = glIsEnabled(GL_LIGHTING);
	static bool material_color = glIsEnabled(GL_COLOR_MATERIAL);
	static bool textures = glIsEnabled(GL_TEXTURE_2D);
	static bool fog = glIsEnabled(GL_FOG);
	static bool antialias = glIsEnabled(GL_LINE_SMOOTH);
	ImGui::Text("Use this tab to enable/disable openGL characteristics");

	if (ImGui::TreeNode("Depth test")) {
		if (ImGui::Checkbox("Enabled##DT Enabled", &depth_test)) {
			if (depth_test)			glEnable(GL_DEPTH_TEST);
			else					glDisable(GL_DEPTH_TEST);
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Face culling")) {
		if (ImGui::Checkbox("Enabled##FC Enabled", &face_culling)) {
			if (face_culling)		glEnable(GL_CULL_FACE);
			else					glDisable(GL_CULL_FACE);
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Lighting")) {
		if (ImGui::Checkbox("Enabled##L Enabled", &lighting)) {
			if (lighting)			glEnable(GL_LIGHTING);
			else					glDisable(GL_LIGHTING);
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Material color")) {
		if (ImGui::Checkbox("Enabled##M Enabled", &material_color)) {
			if (material_color)		glEnable(GL_COLOR_MATERIAL);
			else					glDisable(GL_COLOR_MATERIAL);
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Textures")) {
		if (ImGui::Checkbox("Enabled##T Enabled", &textures)) {
			if (textures)			glEnable(GL_TEXTURE_2D);
			else					glDisable(GL_TEXTURE_2D);
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Fog")) {
		static float fog_distance = 0.5f;
		if (ImGui::Checkbox("Enabled##F Enabled", &fog)) {
			if (fog)				glEnable(GL_FOG);
			else					glDisable(GL_FOG);

			if (fog) {
				GLfloat fog_color[4] = { 0.8f, 0.8f, 0.8f, 0.0f };
				glFogfv(GL_FOG_COLOR, fog_color);
				glFogf(GL_FOG_DENSITY, fog_distance);
			}
		}

		if (ImGui::SliderFloat("##Fog density", &fog_distance, 0.0f, 1.0f, "Fog density: %.2f"))
			glFogf(GL_FOG_DENSITY, fog_distance);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Antialias")) {
		if (ImGui::Checkbox("Enabled##A Enabled", &antialias)) {
			if (antialias)			glEnable(GL_LINE_SMOOTH);
			else					glDisable(GL_LINE_SMOOTH);
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Wireframe")) {
		ImGui::Checkbox("Enabled##WF Enabled", &App->scene->global_wireframe);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Normals")) {
		ImGui::Checkbox("Enabled##N Enabled", &App->scene->global_normals);
		ImGui::TreePop();
	}
}

void PanelConfiguration::DrawWindowConfigLeaf() const
{

	Window* window = App->window->main_window;
	if (ImGui::SliderFloat("##Brightness", &window->brightness, 0, 1.0f, "Brightness: %.2f"))
		App->window->setBrightness(window->brightness);

	bool width_mod, height_mod = false;
	width_mod = ImGui::SliderInt("##Window width", &window->width, MIN_WINDOW_WIDTH, MAX_WINDOW_WIDTH, "Width: %d");
	height_mod = ImGui::SliderInt("###Window height", &window->height, MIN_WINDOW_HEIGHT, MAX_WINDOW_HEIGHT, "Height: %d");

	if (width_mod || height_mod)
		App->window->setSize(window->width, window->height);

	// Refresh rate
	ImGui::Text("Refresh Rate %i", (int)ImGui::GetIO().Framerate);
	//Bools
	if (ImGui::Checkbox("Fullscreen", &window->fullscreen))
		App->window->setFullscreen(window->fullscreen);
	ImGui::SameLine();
	if (ImGui::Checkbox("Resizable", &window->resizable))
		App->window->setResizable(window->resizable);
	if (ImGui::Checkbox("Borderless", &window->borderless))
		App->window->setBorderless(window->borderless);
	ImGui::SameLine();
	if (ImGui::Checkbox("FullDesktop", &window->fulldesk))
		App->window->setFullDesktop(window->fulldesk);
}

void PanelConfiguration::DrawHardwareLeaf() const
{

	//CPUs
	ImGui::Text("CPUs");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0, 255, 0, 255), "%i", SDL_GetCPUCount());

	// RAM
	ImGui::Text("System RAM");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0, 255, 0, 255), "%i Gb", SDL_GetSystemRAM());

	// Caps
	ImGui::Text("Caps:");
	ImGui::SameLine();
	if (SDL_HasRDTSC())
		ImGui::TextColored(ImVec4(0, 255, 0, 255), "RDTSC, ");
	ImGui::SameLine();
	if (SDL_HasMMX())
		ImGui::TextColored(ImVec4(0, 255, 0, 255), "MMX, ");
	ImGui::SameLine();
	if (SDL_HasSSE())
		ImGui::TextColored(ImVec4(0, 255, 0, 255), "SSE, ");
	ImGui::SameLine();
	if (SDL_HasSSE2())
		ImGui::TextColored(ImVec4(0, 255, 0, 255), "SSE2, ");
	if (SDL_HasSSE3())
		ImGui::TextColored(ImVec4(0, 255, 0, 255), "SSE3, ");
	ImGui::SameLine();
	if (SDL_HasSSE41())
		ImGui::TextColored(ImVec4(0, 255, 0, 255), "SSE41, ");
	ImGui::SameLine();
	if (SDL_HasSSE42())
		ImGui::TextColored(ImVec4(0, 255, 0, 255), "SSE42, ");
	ImGui::SameLine();
	if (SDL_HasAVX())
		ImGui::TextColored(ImVec4(0, 255, 0, 255), "AVX.");
	ImGui::SameLine();

	ImGui::Separator();
	//GPU
	ImGui::Text("Caps:");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0, 255, 0, 255), "%s", glGetString(GL_VENDOR));
	ImGui::Text("Brand:");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0, 255, 0, 255), "%s", glGetString(GL_RENDERER));
	ImGui::Text("VRAM Budget:");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0, 255, 0, 255), "%f Mb", getTotalVRAMMb_NVIDIA());
	ImGui::Text("VRAM Usage:");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0, 255, 0, 255), "%f Mb", getTotalVRAMMb_NVIDIA() - getAvaliableVRAMMb_NVIDIA());
	ImGui::Text("VRAM Avaliable:");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0, 255, 0, 255), "%f Mb", getAvaliableVRAMMb_NVIDIA());
	ImGui::Text("VRAM Reserved:");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0, 255, 0, 255), "%f Mb", 0);
}

void PanelConfiguration::DrawApplicationLeaf() const
{
	ImGui::Text("App name: Project-Atlas");
	ImGui::Text("Organization: UPC CITM");
	char title[25];
	sprintf_s(title, 25, "Framerate %.1f", App->fps_log[App->fps_log.size() - 1]);
	ImGui::PlotHistogram("##framerate", &App->fps_log[0], App->fps_log.size(), 0, title, 0.0f, 100.0f, ImVec2(310, 100));
	sprintf_s(title, 25, "Milliseconds %.1f", App->ms_log[App->ms_log.size() - 1]);
	ImGui::PlotHistogram("##milliseconds", &App->ms_log[0], App->ms_log.size(), 0, title, 0.0f, 100.0f, ImVec2(310, 100));

	//ImGui::PopFont();
}

void PanelConfiguration::DrawEditorPreferencesLeaf() const {

	static float camera_speed = 2.5f;
	if (ImGui::InputFloat("Camera speed", &camera_speed))
		App->camera->editor_cam_speed = camera_speed;


	static float camera_rotation_speed = 0.25f;
	if (ImGui::InputFloat("Camera rotation speed", &camera_rotation_speed))
		App->camera->editor_cam_rot_speed = camera_rotation_speed;

	static float near_plane_distance = INIT_N_PLANE;
	static float far_plane_distance = INIT_F_PLANE;

	if (ImGui::InputFloat("Camera near plane distance", &near_plane_distance))
		App->camera->editor_camera->setPlaneDistance(near_plane_distance, far_plane_distance);
	if (ImGui::InputFloat("Camera far plane distance", &far_plane_distance))
		App->camera->editor_camera->setPlaneDistance(near_plane_distance, far_plane_distance);


}

void PanelConfiguration::DrawResourcesLeaf() const
{
	static float refresh_ratio = 1;
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
	if (ImGui::InputFloat("Refresh ratio (seconds)", &refresh_ratio)) {
		App->resources->setRefreshRatio(refresh_ratio * 1000);
	}
}

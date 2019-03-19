#include "PanelAbout.h"
#include "Application.h"

#include "glew-2.1.0/include/GL/glew.h"
#include "Assimp/include/version.h"

PanelAbout::PanelAbout(const char * name):Panel(name)
{
}

PanelAbout::~PanelAbout()
{
}

void PanelAbout::Draw()
{
	ImGui::Begin("About", &active);

	ImGui::Text("Project Atlas");
	ImGui::Separator();
	ImGui::Text("An engine to make videogames");
	ImGui::Text("By the students of Project 3 (2018-2019) at CITM ");
	ImGui::Text("Project Atlas is licensed under the MIT License.\n");
	ImGui::Separator();

	ImGui::Text("Libraries used:");
	ImGui::Text("Assimp %i", ASSIMP_API::aiGetVersionMajor());
	ImGui::SameLine();
	if (ImGui::Button("Learn more##assimp"))
		App->requestBrowser("http://www.assimp.org/");
	ImGui::Text("Glew %s", glewGetString(GLEW_VERSION));
	ImGui::SameLine();
	if (ImGui::Button("Learn more##glew"))
		App->requestBrowser("http://glew.sourceforge.net/");
	ImGui::Text("DevIL 1.8.0");
	ImGui::SameLine();
	if (ImGui::Button("Learn more##devil"))
		App->requestBrowser("http://openil.sourceforge.net/");
	ImGui::Text("MathGeoLib ? version");
	ImGui::SameLine();
	if (ImGui::Button("Learn more##mathgeolib"))
		App->requestBrowser("http://clb.demon.fi/MathGeoLib/");
	SDL_version compiled;
	SDL_GetVersion(&compiled);
	ImGui::Text("SDL %d.%d.%d", compiled.major, compiled.major, compiled.patch);
	ImGui::SameLine();
	if (ImGui::Button("Learn more##sdl"))
		App->requestBrowser("https://wiki.libsdl.org/FrontPage");
	ImGui::Text("OpenGL %s", glGetString(GL_VERSION));
	ImGui::Text("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	ImGui::SameLine();
	if (ImGui::Button("Learn more##opngl"))
		App->requestBrowser("https://www.opengl.org/");
	ImGui::Text("Parson");
	ImGui::SameLine();
	if (ImGui::Button("Learn more##parson"))
		App->requestBrowser("https://github.com/kgabis/parson");
	ImGui::Text("PCG Random");
	ImGui::SameLine();
	if (ImGui::Button("Learn more##pcg_random"))
		App->requestBrowser("http://www.pcg-random.org");
	ImGui::Text("ImGuizmo");
	ImGui::SameLine();
	if (ImGui::Button("Learn more##imguizmo"))
		App->requestBrowser("https://github.com/CedricGuillemet/ImGuizmo");
	ImGui::Text("ImGuiColorTextEdit");
	ImGui::SameLine();
	if (ImGui::Button("Learn more##texteditor"))
		App->requestBrowser("https://github.com/BalazsJako/ImGuiColorTextEdit");
	ImGui::Text("Wren");
	ImGui::SameLine();
	if (ImGui::Button("Learn more##wren"))
		App->requestBrowser("http://wren.io/");
	ImGui::End();
}

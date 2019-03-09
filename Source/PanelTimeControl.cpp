#include "PanelTimeControl.h"
#include "Application.h"
#include "ModuleTimeManager.h"
#include "ModuleUI.h"
#include "Material.h"

PanelTimeControl::PanelTimeControl(const char * name, bool active):Panel(name, active)
{
}

PanelTimeControl::~PanelTimeControl()
{
}

void PanelTimeControl::Draw()
{
	ImGui::Begin("Time control", &active);

	int w, h;
	App->gui->ui_textures[PLAY]->getSize(w, h);
	if (ImGui::ImageButton((void*)App->gui->ui_textures[PLAY]->getGLid(), ImVec2(w, h), nullptr, ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, App->time->getGameState() == PLAYING ? 1.0f : 0.0f)))
		App->time->Play();

	ImGui::SameLine();
	App->gui->ui_textures[PAUSE]->getSize(w, h);
	if (ImGui::ImageButton((void*)App->gui->ui_textures[PAUSE]->getGLid(), ImVec2(w, h), nullptr, ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, App->time->getGameState() == PAUSED ? 1.0f : 0.0f)))
		App->time->Pause();

	ImGui::SameLine();
	App->gui->ui_textures[STOP]->getSize(w, h);
	if (ImGui::ImageButton((void*)App->gui->ui_textures[STOP]->getGLid(), ImVec2(w, h), nullptr, ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, App->time->getGameState() == STOPPED ? 1.0f : 0.0f)))
		App->time->Stop();


	static int advance_frames = 1;
	static float time_scale = 1;

	ImGui::SameLine();
	App->gui->ui_textures[ADVANCE]->getSize(w, h);
	if (ImGui::ImageButton((void*)App->gui->ui_textures[ADVANCE]->getGLid(), ImVec2(w, h), nullptr, ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, 0.0f)))
		App->time->Advance(advance_frames);

	ImGui::Text("Advance frames:");
	ImGui::InputInt("##Advance frames", &advance_frames);

	ImGui::Text("Time scale:");
	if (ImGui::InputFloat("##Time scale", &time_scale))
		App->time->setTimeScale(time_scale);


	ImGui::Text("%f seconds in real time clock", App->time->getRealTime() / 1000);
	ImGui::Text("%f ms delta_time", App->time->getDeltaTime());
	ImGui::Text("%f seconds in game time clock", App->time->getGameTime() / 1000);
	ImGui::Text("%f ms game delta_time", App->time->getGameDeltaTime());
	ImGui::Text("%i frames", App->time->getFrameCount());




	ImGui::End();
}

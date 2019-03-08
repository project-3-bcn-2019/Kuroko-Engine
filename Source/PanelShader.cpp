#include "Application.h"
#include "ModuleUI.h"
#include "PanelShader.h"
#include "ModuleScripting.h"
#include "ModuleInput.h"

#include <fstream>


PanelShader::PanelShader(const char* name, bool _active): Panel(name,_active)
{
	shader_editor.SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
	open_shader_path = "";

	std::ifstream t(open_shader_path.c_str());
	if (t.good())
	{
		std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
		shader_editor.SetText(str);
	}
}


PanelShader::~PanelShader()
{
}

void PanelShader::Draw()
{
	if (App->scripting->edited_scripts.find(open_shader_path) != App->scripting->edited_scripts.end())
		App->scripting->edited_scripts.at(open_shader_path) = shader_editor.GetText();
	else
		App->scripting->edited_scripts.insert(std::make_pair(open_shader_path, shader_editor.GetText()));

	App->gui->disable_keyboard_control = true; // Will disable keybord control forever
	ImGui::PushFont(App->gui->ui_fonts[IMGUI_DEFAULT]);
	auto cpos = shader_editor.GetCursorPosition();

	ImGui::Begin("Shader Editor", &active, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
	ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save"))
			{
				auto textToSave = shader_editor.GetText();
				App->fs.SetFileString(open_shader_path.c_str(), textToSave.c_str());
			}
			if (ImGui::MenuItem("Quit", "Alt-F4")) {
				// Exit or something
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			bool ro = shader_editor.IsReadOnly();
			if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
				shader_editor.SetReadOnly(ro);
			ImGui::Separator();

			if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && shader_editor.CanUndo()))
				shader_editor.Undo();
			if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && shader_editor.CanRedo()))
				shader_editor.Redo();

			ImGui::Separator();

			if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, shader_editor.HasSelection()))
				shader_editor.Copy();
			if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && shader_editor.HasSelection()))
				shader_editor.Cut();
			if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && shader_editor.HasSelection()))
				shader_editor.Delete();
			if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
				shader_editor.Paste();

			ImGui::Separator();

			if (ImGui::MenuItem("Select all", nullptr, nullptr))
				shader_editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(shader_editor.GetTotalLines(), 0));

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Dark palette"))
				shader_editor.SetPalette(TextEditor::GetDarkPalette());
			if (ImGui::MenuItem("Light palette"))
				shader_editor.SetPalette(TextEditor::GetLightPalette());
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	ImGui::BeginChild("Editor",ImVec2(500,600),true);
	ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, shader_editor.GetTotalLines(),
		shader_editor.IsOverwrite() ? "Ovr" : "Ins",
		shader_editor.CanUndo() ? "*" : " ",
		shader_editor.GetLanguageDefinition().mName.c_str(), open_shader_path.c_str());
	
	


	TextEditor::CommandKeys c_keys;
	c_keys.ctrl = (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT);
	c_keys._X = App->input->GetKey(SDL_SCANCODE_X) == KEY_DOWN;
	c_keys._Y = App->input->GetKey(SDL_SCANCODE_Y) == KEY_DOWN;
	c_keys._Z = App->input->GetKey(SDL_SCANCODE_Z) == KEY_DOWN;
	c_keys._C = App->input->GetKey(SDL_SCANCODE_C) == KEY_DOWN;
	c_keys._V = App->input->GetKey(SDL_SCANCODE_V) == KEY_DOWN;

	shader_editor.Render("TextEditor", App->gui->ui_fonts[IMGUI_DEFAULT], c_keys);

	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginGroup();
	ImGui::Text("Shader Options");
	ImVec2 windowPos = ImGui::GetWindowPos();
	ImVec2 optionsPos = { 515, 65};
	ImGui::GetWindowDrawList()->AddLine({ windowPos.x + optionsPos.x,windowPos.y+ optionsPos.y }, ImVec2(windowPos.x+1000, windowPos.y + optionsPos.y), IM_COL32(100, 100, 100, 255), 2.0f);

	ImGui::NewLine();

	ImGui::Text("Shader Type:");
	ImGui::SameLine();
	ImGui::Button("Vertex");
	ImGui::SameLine();
	ImGui::Button("Fragment");

	ImGui::Text("Set Uniform variables");
	ImVec2 uniformPos = { 515,200 };
	ImGui::GetWindowDrawList()->AddLine({ windowPos.x + uniformPos.x,windowPos.y + uniformPos.y }, ImVec2(windowPos.x + 1000, windowPos.y + uniformPos.y), IM_COL32(100, 100, 100, 255), 2.0f);

	ImGui::EndGroup();



	ImGui::PopFont();
	ImGui::End();
}

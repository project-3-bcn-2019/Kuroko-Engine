#include "Application.h"
#include "ModuleUI.h"
#include "PanelShader.h"
#include "ModuleScripting.h"
#include "ModuleInput.h"
#include "ModuleShaders.h"
#include "Applog.h"

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

	current_shader = nullptr;
	selected_type = uniform_types[0];
}


PanelShader::~PanelShader()
{
}

void PanelShader::Draw()
{
	//if (App->scripting->edited_scripts.find(open_shader_path) != App->scripting->edited_scripts.end())
	//	App->scripting->edited_scripts.at(open_shader_path) = shader_editor.GetText();
	//else
		/*App->scripting->edited_scripts.insert(std::make_pair(open_shader_path, shader_editor.GetText()));*/

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

	ImGui::BeginChild("Editor",ImVec2(500,600),true, ImGuiWindowFlags_HorizontalScrollbar);
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
	ImVec2 optionsPos = { 515 - ImGui::GetScrollX(), 65 - ImGui::GetScrollY() };
	ImGui::GetWindowDrawList()->AddLine({ windowPos.x + optionsPos.x,windowPos.y+ optionsPos.y }, ImVec2(windowPos.x+1000, windowPos.y + optionsPos.y), IM_COL32(100, 100, 100, 255), 2.0f);

	ImGui::NewLine();

	//Shader selector-------------------------------------
	std::string shader_name;
	if (current_shader)
	{
		shader_name = current_shader->name;
	}
	else
	{
		shader_name = "New Shader";
	}


	ImGui::Text("Current Shader:");
	ImGui::SameLine();
	ImGui::PushItemWidth(200.0f);
	ImGui::PushID("Shader name");
	if (ImGui::BeginCombo("", shader_name.c_str()))
	{
		for (int i=0;i<App->shaders->shaders.size()+1;++i)
		{
			bool selected = false;
			if (i < App->shaders->shaders.size())
			{
				if (ImGui::Selectable(App->shaders->shaders[i]->name.c_str(), &selected))
				{
					current_shader = App->shaders->shaders[i];
					shader_editor.SetText(current_shader->script);
				}
			}
			else
			{
				if (ImGui::Selectable("New Shader", &selected))
				{
					current_shader = nullptr;
					std::string str=" ";
					shader_editor.SetText(str);
					fragment = vertex = false;
				}
			}
		}
		ImGui::EndCombo();
	}
	ImGui::PopID();
	ImGui::PopItemWidth();


	if (current_shader != nullptr)
	{
		if (current_shader->type == VERTEX)
		{
			vertex = true;
			fragment = false;
		}
		else
		{
			vertex = false;
			fragment = true;
		}
	}
	


	ImGui::Text("Shader Type:");
	ImGui::SameLine();
	if (ImGui::Checkbox("Vertex",&vertex))
	{
		fragment = false;
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Fragment", &fragment))
	{
		vertex = false;
	}

	ImGui::Text("New Name:");
	ImGui::SameLine();

	ImGui::PushID("new name");
	ImGui::PushItemWidth(150.0f);
	ImGui::InputText("", shaderName, 256);
	ImGui::PopItemWidth();
	ImGui::PopID();
	ImGui::SameLine();

	ImGui::Dummy(ImVec2(0.0f, 52.0f));

	ImGui::Text("Uniform variables");
	ImVec2 uniformPos = { 515 - ImGui::GetScrollX(),200 - ImGui::GetScrollY() };
	ImGui::GetWindowDrawList()->AddLine({ windowPos.x + uniformPos.x,windowPos.y + uniformPos.y }, ImVec2(windowPos.x + 1000, windowPos.y + uniformPos.y), IM_COL32(100, 100, 100, 255), 2.0f);

	ImGui::NewLine();
	
	//Uniform Creator
		
	ImGui::Text("Name:");
	ImGui::SameLine();
	
	ImGui::PushID("name");
	ImGui::PushItemWidth(150.0f);
	ImGui::InputText("", uniform_name, 256);
	ImGui::PopItemWidth();
	ImGui::PopID();
	ImGui::SameLine();
	
	ImGui::Text("Size:");
	ImGui::SameLine();

	ImGui::PushID("size");
	ImGui::PushItemWidth(75.0f);
	ImGui::InputInt("", &uniform_size,1,100,ImGuiInputTextFlags_AlwaysInsertMode);
	ImGui::PopItemWidth();
	ImGui::PopID();
	ImGui::SameLine();

	ImGui::PushItemWidth(50.0f);
	ImGui::PushID("UniformTypes");
	if (ImGui::BeginCombo("", selected_type))
	{
		for (int i = 0; i < uniform_types.size(); ++i)
		{
			bool selected = false;
			
			if (ImGui::Selectable(uniform_types[i], &selected))
			{
				selected_type = uniform_types[i];
			}
		}
		ImGui::EndCombo();
	}
	ImGui::PopID();
	ImGui::PopItemWidth();

	if (ImGui::Button("Uniform ++"))
	{
		AddUniform();
	}
	

	if (current_shader)
	{
		ImGui::BeginChild("Uniform Var", ImVec2(400, 300), true);
		for (int i = 0; i < current_shader->uniforms.size(); ++i)
		{
			std::string uniform_info = current_shader->uniforms[i]->name + " " + current_shader->uniforms[i]->stringType;
			ImGui::Text(uniform_info.c_str());
		}
	}
	else
	{
		ImGui::BeginChild("Uniform Var", ImVec2(400, 300), true);
		for (int i = 0; i < shader_uniforms.size(); ++i)
		{
			std::string uniform_info = shader_uniforms[i]->name + " " + shader_uniforms[i]->stringType;
			ImGui::Text(uniform_info.c_str());
		}
	}
	



	ImGui::EndChild();

	if (ImGui::Button("Save"))
	{
		SaveShader(current_shader);
	}
	
	ImGui::EndGroup();

	

	ImGui::PopFont();
	ImGui::End();
}

void PanelShader::SaveShader(Shader* shader)
{
	if (shader)
	{
		std::string saveCopy = shader->script;
		shader->script = shader_editor.GetText();

		if (App->shaders->CompileShader(shader))
		{
			//App.shader.recompilePrograms();
		}
		else
		{
			app_log->AddLog("Error modifing shader, check the file for errors");
		}

	}
	else
	{
		if (vertex || fragment)
		{
			Shader* aux_shader = new Shader(vertex == true ? VERTEX : FRAGMENT);
			
			aux_shader->script = shader_editor.GetText();

			aux_shader->name = shaderName;
			aux_shader->uniforms = shader_uniforms;
			if (App->shaders->CompileShader(aux_shader))
			{
				App->shaders->shaders.push_back(aux_shader);
				shader_uniforms.clear();
				shader_editor.SetText(" ");
				vertex = fragment = false;
				
			}
			else
			{
				RELEASE(aux_shader);
			}
		}
	}

}

void PanelShader::AddUniform()
{
	Uniform* aux_uniform = new Uniform(uniform_name, selected_type, uniform_size);
	if (current_shader)
	{
		current_shader->uniforms.push_back(aux_uniform);
	}
	else
	{
		shader_uniforms.push_back(aux_uniform);
	}
	
	

	selected_type = uniform_types[0];
	uniform_size = 1;
}

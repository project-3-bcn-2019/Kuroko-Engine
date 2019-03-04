#include "PanelAssetsWin.h"
#include "Application.h"
#include "ModuleResourcesManager.h"
#include "ModuleInput.h"
#include "ModuleUI.h"
#include "Material.h"
#include "ScriptData.h"

#include "ResourceTexture.h"
#include "ResourceScript.h"

#include <experimental/filesystem>
#include <fstream>


PanelAssetsWin::PanelAssetsWin(const char * name, bool active): Panel(name, active)
{
}

PanelAssetsWin::~PanelAssetsWin()
{
}

void PanelAssetsWin::Draw()
{
	ImGui::Begin("Assets Window", &active);
	int element_size = 64;
	std::string path, name, extension;

	int column_num = (int)trunc(ImGui::GetWindowSize().x / (element_size + 20));
	static bool item_hovered = false;

	if (column_num != 0)
	{
		int count = 0;
		int iteration = 0;

		if (ImGui::ImageButton((void*)App->gui->ui_textures[RETURN_ICON]->getGLid(), ImVec2(24, 17)))
		{
			if (!App->fs.getPath(asset_window_path))
				asset_window_path = ASSETS_FOLDER;
		}

		ImGui::SameLine();
		ImGui::Text(asset_window_path.c_str());

		using std::experimental::filesystem::directory_iterator;
		for (auto& it : directory_iterator(asset_window_path))
		{
			extension = it.path().generic_string();
			App->fs.getExtension(extension);
			if (extension != ".meta")
				count++;
		}

		if (count == 0)
		{
			ImGui::End();
			DrawAssetInspector();
			return;
		}

		else if (count < column_num) column_num = count;
		count = 0;

		ImGui::Columns(column_num, (std::to_string(iteration) + " asset columns").c_str(), false);

		for (auto& it : directory_iterator(asset_window_path)) {

			path = name = extension = it.path().generic_string();	// Separate path, name and extension	
			App->fs.getExtension(extension);
			App->fs.getPath(path);
			App->fs.getFileNameFromPath(name);

			if (extension == ".meta")
				continue;

			if (count == column_num)
			{
				ImGui::NewLine();
				iteration++;
				count = 0;
				ImGui::Columns(column_num, (std::to_string(iteration) + " asset columns").c_str(), false);
			}
			count++;

			if (column_num > 1)
				ImGui::SetColumnWidth(ImGui::GetColumnIndex(), element_size + 20);

			bool draw_caution = false;
			bool draw_warning = false;
			std::string error_message;

			if (it.status().type() == std::experimental::filesystem::v1::file_type::directory)
			{

				if (ImGui::IsMouseDoubleClicked(0))
				{
					ImGui::ImageButton((void*)App->gui->ui_textures[FOLDER_ICON]->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f));
					if (ImGui::IsItemHovered())
						asset_window_path = it.path().generic_string();
				}
				else {
					if (ImGui::ImageButton((void*)App->gui->ui_textures[FOLDER_ICON]->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f)))
						selected_asset = it.path().generic_string();
					else if (ImGui::IsItemHovered())
						item_hovered = true;
				}
			}
			else
			{
				const char* type = App->resources->assetExtension2type(extension.c_str());

				if (type == "3dobject")
				{

					if (ImGui::IsMouseDoubleClicked(0)) {
						ImGui::ImageButton((void*)App->gui->ui_textures[OBJECT_ICON]->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f));
						if (ImGui::IsItemHovered())
							App->resources->Load3dObjectToScene(it.path().generic_string().c_str());
					}
					else {
						if (ImGui::ImageButton((void*)App->gui->ui_textures[OBJECT_ICON]->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f)))
							selected_asset = it.path().generic_string();
						else if (ImGui::IsItemHovered())
							item_hovered = true;
					}
				}
				else if (type == "texture")
				{
					ResourceTexture* res_tex = (ResourceTexture*)App->resources->getResource(App->resources->getResourceUuid(it.path().generic_string().c_str()));
					if (res_tex) {
						res_tex->drawn_in_UI = true;
						if (!res_tex->IsLoaded())
							res_tex->LoadToMemory();
						if (ImGui::ImageButton((void*)res_tex->texture->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f)))
							selected_asset = it.path().generic_string();
						else if (ImGui::IsItemHovered())
							item_hovered = true;
					}


				}
				else if (type == "prefab") {

					if (ImGui::IsMouseDoubleClicked(0)) {
						ImGui::ImageButton((void*)App->gui->ui_textures[PREFAB_ICON]->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f));
						if (ImGui::IsItemHovered())
							App->scene->AskPrefabLoadFile((char*)it.path().generic_string().c_str(), float3(0, 0, 0), float3(0, 0, 0));
					}
					else {
						if (ImGui::ImageButton((void*)App->gui->ui_textures[PREFAB_ICON]->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f)))
							selected_asset = it.path().generic_string();
						else if (ImGui::IsItemHovered())
							item_hovered = true;
					}
				}

				else if (type == "scene")
				{
					if (ImGui::IsMouseDoubleClicked(0)) {
						ImGui::ImageButton((void*)App->gui->ui_textures[SCENE_ICON]->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f));
						if (ImGui::IsItemHovered())
							App->scene->AskSceneLoadFile((char*)it.path().generic_string().c_str());
					}
					else {
						if (ImGui::ImageButton((void*)App->gui->ui_textures[SCENE_ICON]->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f)))
							selected_asset = it.path().generic_string();
						else if (ImGui::IsItemHovered())
							item_hovered = true;
					}
				}

				else if (type == "script")
				{
					ResourceScript* res = (ResourceScript*)App->resources->getResource(App->resources->getResourceUuid(it.path().generic_string().c_str()));

					if (res && res->IsInvalid())
					{
						draw_warning = true;
						error_message += "Compile error in imported script";
					}

					if (ImGui::IsMouseDoubleClicked(0)) {
						ImGui::ImageButton((void*)App->gui->ui_textures[SCRIPT_ICON]->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f));
						if (ImGui::IsItemHovered())
						{
							App->gui->open_script_path = it.path().generic_string();
							App->gui->open_tabs[SCRIPT_EDITOR] = true;

							if (App->scripting->edited_scripts.find(App->gui->open_script_path) != App->scripting->edited_scripts.end())
								App->gui->script_editor.SetText(App->scripting->edited_scripts.at(App->gui->open_script_path));
							else
							{
								std::ifstream t(App->gui->open_script_path.c_str());
								if (t.good())
								{
									std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
									App->gui->script_editor.SetText(str);
								}
							}
						}
					}
					else {
						if (ImGui::ImageButton((void*)App->gui->ui_textures[SCRIPT_ICON]->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f)))
							selected_asset = it.path().generic_string();
						else if (ImGui::IsItemHovered())
							item_hovered = true;
					}
				}
			}

			if (draw_warning || draw_caution)
			{
				ImGui::Image((void*)App->gui->ui_textures[draw_warning ? WARNING_ICON : CAUTION_ICON]->getGLid(), ImVec2(16, 16));

				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text(error_message.c_str());
					ImGui::EndTooltip();
				}

				ImGui::SameLine();
			}

			ImGui::TextWrapped(name.c_str());
			ImGui::NextColumn();
		}
		ImGui::Columns(1);
	}

	if (ImGui::IsWindowHovered())
	{
		if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
			selected_asset = "";
		else if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_DOWN && !item_hovered)
			ImGui::OpenPopup("##asset window context menu");
	}
	item_hovered = false;

	static bool name_script = false;
	if (ImGui::BeginPopup("##asset window context menu"))
	{
		if (ImGui::Button("Add script"))
			name_script = true;

		ImGui::EndPopup();
	}

	ImGui::End();

	if (name_script) {
		App->gui->disable_keyboard_control = true;
		ImGui::Begin("Script Name", &name_script);
		//ImGui::PushFont(ui_fonts[REGULAR]);

		static char rename_buffer[64];
		ImGui::InputText("Create as...", rename_buffer, 64);
		ImGui::SameLine();
		if (ImGui::Button("Create")) {
			std::string script_name = rename_buffer;
			std::string full_path = asset_window_path + "/" + script_name + ".wren";
			App->fs.CreateEmptyFile(full_path.c_str());
			App->gui->open_script_path = full_path;
			std::string file_initial_text;
			file_initial_text =
				"\nimport \"ObjectLinker\" for ObjectLinker,\nEngineComunicator,\nInputComunicator\n"
				"\n//For each var you declare, remember to create"
				"\n//		setters [varname=(v) { __varname = v }]"
				"\n//		and getters [varname { __varname }]"
				"\n//The construct method is mandatory, do not erase!"
				"\n//The import statement at the top og the cript is mandatory, do not erase!"
				"\n//Be careful not to overwrite the methods declared in Game/ScriptingAPI/ObjectLinker.wren"
				"\n//[gameObject] is a reserved identifier for the engine, don't use it for your own variables"
				"\n\nclass " + script_name + " is ObjectLinker{"
				"\n\nconstruct new(){}"
				"\n\n Start() {}"
				"\n\n Update() {}"
				"\n}";
			App->gui->script_editor.SetText(file_initial_text);
			App->fs.SetFileString(App->gui->open_script_path.c_str(), file_initial_text.c_str());
			App->gui->open_tabs[SCRIPT_EDITOR] = true;
			for (int i = 0; i < 64; i++)
				rename_buffer[i] = '\0';
			name_script = false;
		}
		//ImGui::PopFont();
		ImGui::End();
	}

	DrawAssetInspector();
}

void PanelAssetsWin::DrawAssetInspector()
{
	ImGui::Begin("Asset inspector", nullptr);

	if (!selected_asset.empty())
	{
		std::string name, extension;
		extension = name = selected_asset;
		App->fs.getExtension(extension);
		App->fs.getFileNameFromPath(name);

		ImGui::Text(name.c_str());

		if (extension.empty())  // is directory
		{
			ImGui::Text("type: directory");
			ImGui::End();
			return;
		}
		else if (extension == ".scene")
		{
			ImGui::Text("type: scene");
			ImGui::End();
			return;
		}

		const char* type = App->resources->assetExtension2type(extension.c_str());
		if (type == "3dobject")
			ImGui::Text("type: 3D object");
		else
			ImGui::Text("type: %s", type);

		Resource* res = App->resources->getResource(App->resources->getResourceUuid(selected_asset.c_str()));
		if (res) {
			ImGui::Text("Used by %s components", std::to_string(res->components_used_by).c_str());
			if (res->IsLoaded())		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Loaded");
			else					ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Unloaded");
		}
		else
			return;

		if (type == "texture")
		{
			ResourceTexture* res_tex = (ResourceTexture*)res;
			if (res_tex) {
				res_tex->drawn_in_UI = true;
				if (!res_tex->IsLoaded())
					res_tex->LoadToMemory();

				int size_x, size_y;
				res_tex->texture->getSize(size_x, size_y);
				ImGui::Image((void*)res_tex->texture->getGLid(), ImVec2((float)size_x, (float)size_y));

				ImGui::Scrollbar(ImGuiLayoutType_::ImGuiLayoutType_Horizontal);
			}
		}
		else if (type == "script")
		{

			ScriptData* script_data = ((ResourceScript*)res)->getData();

			bool updated = false;

			if (!script_data) {
				ImGui::Text("Script can't be compiled!");
			}
			else {
				for (auto it = script_data->vars.begin(); it != script_data->vars.end(); it++) {

					ImportedVariable* curr = &(*it);
					std::string unique_tag = "##" + curr->getName();

					ImGui::Text(curr->getName().c_str());
					static int type = 0;
					type = curr->getType();
					if (ImGui::Combo((unique_tag + "type").c_str(), &type, "None\0Bool\0String\0Numeral\0"))
					{
						if (type != 0)
							curr->setType((ImportedVariable::WrenDataType)(type));

						updated = true;
					}

					ImGui::SameLine();

					static bool forced_type = false;
					forced_type = curr->isTypeForced();
					if (ImGui::Checkbox(("Forced" + unique_tag + "forced").c_str(), &forced_type))
					{
						if (type != 0)
						{
							curr->setForcedType(forced_type);
							updated = true;
						}
					}

					static bool _public = true;
					_public = curr->isPublic();
					if (ImGui::Checkbox(("Public" + unique_tag + "public").c_str(), &_public))
					{
						curr->setPublic(_public);
						updated = true;
					}
				}

				if (updated)
				{
					for (auto instance = App->scripting->loaded_instances.begin(); instance != App->scripting->loaded_instances.end(); instance++)
					{
						if ((*instance)->class_name == script_data->class_name)
						{
							if ((*instance)->vars.size() == script_data->vars.size());   // should always be true, but to be safe
							for (int i = 0; i < (*instance)->vars.size(); i++)
							{
								(*instance)->vars[i].setType(script_data->vars[i].getType());
								(*instance)->vars[i].setPublic(script_data->vars[i].isPublic());
								(*instance)->vars[i].setForcedType(script_data->vars[i].isTypeForced());
							}
						}
					}
				}
			}
		}
	}

	ImGui::End();
}

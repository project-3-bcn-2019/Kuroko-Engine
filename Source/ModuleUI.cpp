 #include "ModuleUI.h"
#include "Globals.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleWindow.h"
#include "ModuleImporter.h"
#include "ModuleExporter.h"
#include "ModuleCamera3D.h"
#include "ModuleScene.h"
#include "ModuleResourcesManager.h"
#include "ModuleScripting.h"
#include "ModuleInput.h"


#include "Applog.h"
#include "ScriptData.h"

#include "ImGui/imgui_impl_sdl.h"
#include "ImGui/imgui_impl_opengl2.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui.h"

#include "GameObject.h"
#include "ComponentCamera.h"
#include "ComponentRectTransform.h"
#include "Camera.h"

#include "Random.h"
#include "WinItemDialog.h" 

#include "glew-2.1.0\include\GL\glew.h"
#include "SDL\include\SDL_opengl.h"
#include <gl/GL.h>
#include <gl/GLU.h>

#include <experimental/filesystem>
#include <fstream>

// Include Panels
#include "Panel.h"
#include "PanelAnimation.h"
#include "PanelAnimationEvent.h"
#include "PanelHierarchyTab.h"
#include "PanelObjectInspector.h"
#include "PanelAssetsWin.h"
#include "PanelPrimitives.h"
#include "PanelAnimationGraph.h"
#include "PanelSkyboxWin.h"
#include "PanelConfiguration.h"
#include "PanelTimeControl.h"
#include "PanelShader.h"
#include "PanelAbout.h"
#include "PanelCameraMenu.h"
#include "PanelViewports.h"
#include "PanelQuadtreeConfig.h"

#pragma comment( lib, "glew-2.1.0/lib/glew32.lib")
#pragma comment( lib, "glew-2.1.0/lib/glew32s.lib")

#define CAMERA_VIEW_MARGIN_X 25
#define CAMERA_VIEW_MARGIN_Y 45


ModuleUI::ModuleUI(Application* app, bool start_enabled) : Module(app, start_enabled) {
	name = "gui";

	panels.push_back(p_anim = new PanelAnimation("AnimEditor"));
	panels.push_back(p_anim_evt = new PanelAnimationEvent("AnimEvtEditor"));
	panels.push_back(p_hierarchy = new PanelHierarchyTab("Hierarchy", true));
	panels.push_back(p_inspector = new PanelObjectInspector("Object Inspector", true));
	panels.push_back(p_assetswindow = new PanelAssetsWin("Assets", true));
	panels.push_back(p_primitives = new PanelPrimitives("Primitives", true));
	panels.push_back(p_animation_graph = new PanelAnimationGraph("Animation Graph", false));
	panels.push_back(p_configuration = new PanelConfiguration("Configuration", true));
	panels.push_back(p_time_control = new PanelTimeControl("Time Control", true));
	panels.push_back(p_shader_editor = new PanelShader("Shader Editor", false));
	panels.push_back(p_about = new PanelAbout("About"));
	panels.push_back(p_camera_menu = new PanelCameraMenu("Camera Menu"));
	panels.push_back(p_viewports = new PanelViewports("Viewports"));
	panels.push_back(p_quadtree_config = new PanelQuadtreeConfig("Quadtree Configuration"));
	panels.push_back(p_skybox = new PanelSkyboxWin("Skybox"));
}

ModuleUI::~ModuleUI() {
}

bool ModuleUI::Init(const JSON_Object* config) {
	
	SDL_GL_SetSwapInterval(1); // Enable vsync

	// Setup Dear ImGui binding
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();


	ImGui_ImplSDL2_InitForOpenGL(App->window->main_window->window, App->renderer3D->getContext());
	ImGui_ImplOpenGL2_Init();
	
	// Setup style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();
	LoadConfig(config);

	InitializeScriptEditor();

	// If it is a game build, we hide UI
	enabled = !App->is_game;


	return true;
}

bool ModuleUI::Start()
{
	if (!App->is_game)
	{
		io = &ImGui::GetIO();

		ui_textures[PLAY] = (Texture*)App->importer->ImportTexturePointer("Editor textures/Play.png");
		ui_textures[PAUSE] = (Texture*)App->importer->ImportTexturePointer("Editor textures/Pause.png");
		ui_textures[STOP] = (Texture*)App->importer->ImportTexturePointer("Editor textures/Stop.png");
		ui_textures[ADVANCE] = (Texture*)App->importer->ImportTexturePointer("Editor textures/Advance.png");

		ui_textures[GUIZMO_TRANSLATE] = (Texture*)App->importer->ImportTexturePointer("Editor textures/translate.png");
		ui_textures[GUIZMO_ROTATE] = (Texture*)App->importer->ImportTexturePointer("Editor textures/rotate.png");
		ui_textures[GUIZMO_SCALE] = (Texture*)App->importer->ImportTexturePointer("Editor textures/scale.png");
		ui_textures[GUIZMO_LOCAL] = (Texture*)App->importer->ImportTexturePointer("Editor textures/Guizmo_local.png");
		ui_textures[GUIZMO_GLOBAL] = (Texture*)App->importer->ImportTexturePointer("Editor textures/Guizmo_global.png");
		ui_textures[GUIZMO_SELECT] = (Texture*)App->importer->ImportTexturePointer("Editor textures/Guizmo_select.png");

		ui_textures[NO_TEXTURE] = (Texture*)App->importer->ImportTexturePointer("Editor textures/no_texture.png");

		ui_textures[FOLDER_ICON] = (Texture*)App->importer->ImportTexturePointer("Editor textures/folder_icon.png");
		ui_textures[OBJECT_ICON] = (Texture*)App->importer->ImportTexturePointer("Editor textures/object_icon.png");
		ui_textures[SCENE_ICON] = (Texture*)App->importer->ImportTexturePointer("Editor textures/scene_icon.png");
		ui_textures[RETURN_ICON] = (Texture*)App->importer->ImportTexturePointer("Editor textures/return_icon.png");
		ui_textures[SCRIPT_ICON] = (Texture*)App->importer->ImportTexturePointer("Editor textures/script_icon.png");
		ui_textures[PREFAB_ICON] = (Texture*)App->importer->ImportTexturePointer("Editor textures/prefab_icon.png");
		ui_textures[GRAPH_ICON] = (Texture*)App->importer->ImportTexturePointer("Editor textures/graph_icon.png");
		ui_textures[AUDIO_ICON] = (Texture*)App->importer->ImportTexturePointer("Editor textures/audio_icon.png");
		ui_textures[SHADER_ICON] = (Texture*)App->importer->ImportTexturePointer("Editor textures/shader_icon.png");

		ui_textures[CAUTION_ICON] = (Texture*)App->importer->ImportTexturePointer("Editor textures/caution_icon_32.png");
		ui_textures[WARNING_ICON] = (Texture*)App->importer->ImportTexturePointer("Editor textures/warning_icon_32.png");

		ui_fonts[TITLES] = io->Fonts->AddFontFromFileTTF("Fonts/title.ttf", 16.0f);
		ui_fonts[REGULAR] = io->Fonts->AddFontFromFileTTF("Fonts/regular.ttf", 18.0f);
		ui_fonts[IMGUI_DEFAULT] = io->Fonts->AddFontDefault();
		//ui_fonts[REGULAR_BOLD]		= io->Fonts->AddFontFromFileTTF("Fonts/regular_bold.ttf", 18.0f);
		//ui_fonts[REGULAR_ITALIC]		= io->Fonts->AddFontFromFileTTF("Fonts/regular_italic.ttf", 18.0f);
		//ui_fonts[REGULAR_BOLDITALIC]	= io->Fonts->AddFontFromFileTTF("Fonts/regular_bold_italic.ttf", 18.0f);

		open_tabs[BUILD_MENU] = false;

		ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = { 0.1f,0.1f,0.1f,1.0f };
		ImGui::GetStyle().Colors[ImGuiCol_ScrollbarBg] = { 0.05f, 0.05f,0.05f,1.0f };
		ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = { 0.05f, 0.05f,0.05f,1.0f };
		ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = { 0.3f, 0.3f,0.3f,1.0f };

		io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		//io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io->IniFilename = "Settings\\imgui.ini";
		docking_background = true;
	}

	return true;
}

update_status ModuleUI::PreUpdate(float dt) {

	if (!App->is_game)
	{

		// Start the ImGui frame
		ImGui_ImplOpenGL2_NewFrame();

		ImGui_ImplSDL2_NewFrame(App->window->main_window->window);
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	return UPDATE_CONTINUE;
}

update_status ModuleUI::Update(float dt) {

	if (!App->is_game)
	{
		
		InvisibleDockingBegin();
		static bool file_save = false;
		disable_keyboard_control = false;

		if (open_log_menu)
			app_log->Draw("App log", &open_log_menu);

		if (open_tabs[SCRIPT_EDITOR])
			DrawScriptEditor();

		if (open_tabs[BUILD_MENU])
			DrawBuildMenu();

		for (auto it : panels)
		{
			if (it->isActive())
			{
				it->Draw();
			}
		}

		

		for (auto it = App->camera->game_cameras.begin(); it != App->camera->game_cameras.end(); it++)
		{
			if ((*it)->draw_in_UI || (*it) == App->camera->editor_camera)
				DrawCameraViewWindow(*(*it));
		}

		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Quit"))
					App->CLOSE_APP();
				if (ImGui::MenuItem("Import file"))
				{
					std::string file_path = openFileWID();
					std::string extension = file_path;
					App->fs.getExtension(extension);
					App->fs.copyFileTo(file_path.c_str(), ASSETS, extension.c_str());
					app_log->AddLog("%s copied to Assets folder", file_path.c_str());
				}
				if (ImGui::MenuItem("Save scene")) {
					if (App->scene->existingScene())
						App->scene->AskSceneSaveFile((char*)App->scene->getWorkigSceneName().c_str());
					else {
						file_save = true;
					}

				}
				if (ImGui::MenuItem("Save scene as...")) {
					file_save = true;
				}
				if (ImGui::MenuItem("Load scene")) {
					std::string file_path = openFileWID();
					App->scene->AskSceneLoadFile((char*)file_path.c_str());
				}
				if (ImGui::BeginMenu("Configuration")) {
					if (ImGui::MenuItem("Save Configuration"))
						App->SaveConfig();
					if (ImGui::MenuItem("Delete Configuration"))
						App->DeleteConfig();
					if (ImGui::MenuItem("Load Default Configuration"))
						App->LoadDefaultConfig();
					ImGui::EndMenu();
				}

				if (ImGui::MenuItem("Clean library (App will shut down)"))
					App->resources->CleanResources();

				if (ImGui::MenuItem("Make Build"))
				{
					open_tabs[BUILD_MENU] = true;
				}

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("View")) {
				if (ImGui::MenuItem("Hierarchy", NULL, p_hierarchy->isActive()))
					p_hierarchy->toggleActive();
				if (ImGui::MenuItem("Animation Graph", NULL, p_anim->isActive()))
					p_animation_graph->toggleActive();
				if (ImGui::MenuItem("Object Inspector", NULL, p_inspector->isActive()))
					p_inspector->toggleActive();
				if (ImGui::MenuItem("Primitive", NULL, p_primitives->isActive()))
					p_primitives->toggleActive();
				if (ImGui::MenuItem("Configuration", NULL, p_configuration->isActive()))
					p_configuration->toggleActive();
				ImGui::MenuItem("Log", NULL, &open_log_menu);
				if (ImGui::MenuItem("Time control", NULL, p_time_control->isActive()))
					p_time_control->toggleActive();
				if (ImGui::MenuItem("Quadtree", NULL, p_quadtree_config->isActive()))
					p_quadtree_config->toggleActive();
				if (ImGui::MenuItem("Camera Menu", NULL, p_camera_menu->isActive()))
					p_camera_menu->toggleActive();
				if (ImGui::MenuItem("Assets", NULL, p_assetswindow->isActive()))
					p_assetswindow->toggleActive();
				if (ImGui::MenuItem("Skybox", NULL, p_skybox->isActive()))
				{
					p_skybox->toggleActive();
				}
				ImGui::MenuItem("Script Editor", NULL, &open_tabs[SCRIPT_EDITOR]);
				if (ImGui::MenuItem("Shader Editor", NULL, p_shader_editor->isActive()))
					p_shader_editor->toggleActive();
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Help")) {
				if (ImGui::MenuItem("About", NULL, p_about->isActive()))
					p_about->toggleActive();
				if (ImGui::MenuItem("Documentation"))
					App->requestBrowser("https://github.com/Skyway666/Kuroko-Engine/wiki");
				if (ImGui::MenuItem("Download latest"))
					App->requestBrowser("https://github.com/Skyway666/Kuroko-Engine/releases");
				if (ImGui::MenuItem("Report a bug"))
					App->requestBrowser("https://github.com/RustikTie/project-3-bcn/issues");
				ImGui::EndMenu();
			}
/*
			std::string current_viewport = "Viewport: Free camera";

			switch (App->camera->selected_camera->getViewportDir())
			{
			case VP_RIGHT: current_viewport = "Viewport: Right"; break;
			case VP_LEFT: current_viewport = "Viewport: Left"; break;
			case VP_UP: current_viewport = "Viewport: Up"; break;
			case VP_DOWN: current_viewport = "Viewport: Down"; break;
			case VP_FRONT: current_viewport = "Viewport: Front"; break;
			case VP_BACK: current_viewport = "Viewport: Back"; break;
			default: break;
			}

			if (ImGui::BeginMenu(current_viewport.c_str()))
			{
				if (App->camera->selected_camera != App->camera->editor_camera)
				{
					if (ImGui::MenuItem("Free Camera"))
					{
						App->camera->selected_camera->active = false;
						App->camera->selected_camera = App->camera->background_camera = App->camera->editor_camera;
						App->camera->selected_camera->active = true;
					}
				}

				std::string viewport_name;
				for (int i = 0; i < 6; i++)
				{
					if (App->camera->selected_camera != App->camera->viewports[i])
					{
						if (ImGui::MenuItem(App->camera->viewports[i]->getViewportDirString().c_str()))
						{
							App->camera->background_camera->active = false;
							App->camera->background_camera = App->camera->selected_camera = App->camera->viewports[i];
							App->camera->background_camera->active = true;
						}
					}
				}

				if (ImGui::MenuItem("Open viewport menu", nullptr, p_viewports->isActive()))
					p_viewports->toggleActive();

				ImGui::EndMenu();
			}*/

		}
		ImGui::EndMainMenuBar();

		if (file_save) {
			disable_keyboard_control = true;
			ImGui::Begin("Scene Name", &file_save);
			ImGui::PushFont(ui_fonts[REGULAR]);

			static char rename_buffer[64];
			ImGui::InputText("Save as...", rename_buffer, 64);
			ImGui::SameLine();
			if (ImGui::Button("Save")) {
				App->scene->AskSceneSaveFile(rename_buffer);
				for (int i = 0; i < 64; i++)
					rename_buffer[i] = '\0';
				file_save = false;
			}
			ImGui::PopFont();
			ImGui::End();
		}



		InvisibleDockingEnd();
	}
	return UPDATE_CONTINUE;
}

update_status ModuleUI::PostUpdate(float dt) {
	// Rendering
	if (!App->is_game)
	{
		ImGui::Render();

		glViewport(0, 0, (int)io->DisplaySize.x, (int)io->DisplaySize.y);

		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
	}

	return UPDATE_CONTINUE;

}

bool ModuleUI::CleanUp() 
{
	if (!App->is_game)
	{
		for (int i = 0; i < LAST_UI_TEX; i++) { // Cleanup textures
			delete ui_textures[i];
		}
	}

	for (std::list<Panel*>::iterator iterator = panels.begin(); iterator != panels.end(); iterator++)
	{
		if (*iterator != nullptr)
			delete *iterator;
		*iterator = nullptr;
	}
	panels.clear();

	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	return true;
}

void ModuleUI::InitializeScriptEditor()
{

	script_editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Wren());


	//TextEditor::ErrorMarkers markers;
	//markers.insert(std::make_pair<int, std::string>(20, "Example error here:\nInclude file not found: \"TextEditor.h\""));
	//markers.insert(std::make_pair<int, std::string>(41, "Another example error"));
	//script_editor.SetErrorMarkers(markers);

	open_script_path = "";

	std::ifstream t(open_script_path.c_str());
	if (t.good())
	{
		std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
		script_editor.SetText(str);
	}

}

void ModuleUI::DrawCameraViewWindow(Camera& camera)
{
	App->camera->enable_camera_control = false;

	if (FrameBuffer* frame_buffer = camera.getFrameBuffer())
	{
		std::string window_name;

		if (camera.getParent())
			window_name = camera.getParent()->getParent()->getName() + " Camera";
		else
			window_name = camera.getViewportDirString();

		if (App->camera->game_camera == &camera)
			window_name += "(Game camera)";

		ImGui::Begin(window_name.c_str(), &camera.draw_in_UI, ImGuiWindowFlags_NoScrollbar);

		ImVec2 window_pos = ImGui::GetItemRectMin();

		ImVec2 window_size = ImGui::GetWindowSize();
		window_size.x -= CAMERA_VIEW_MARGIN_X;
		window_size.y -= CAMERA_VIEW_MARGIN_Y;

		frame_buffer->changeTexSize(window_size.x, window_size.y);

		if ((ImGui::ImageButton((void*)(camera.draw_depth ? frame_buffer->depth_tex->gl_id : frame_buffer->tex->gl_id), window_size, nullptr, ImVec2(0, 1), ImVec2(1, 0), -1)) && (draw_guizmo ? !ImGuizmo::IsOver() : true))
		{
			if (&camera == App->camera->editor_camera)
			{
				window_pos = ImGui::GetItemRectMin();
				float x = App->input->GetMouseX(); float y = App->input->GetMouseY();
				x = (((x - window_pos.x) / ImGui::GetItemRectSize().x) * 2) - 1;
				y = (((y - window_pos.y) / ImGui::GetItemRectSize().y) * -2) + 1;

				if (GameObject* picked_obj = App->scene->MousePicking(x, y, nullptr))
				{
					if (!App->input->GetKey(SDL_SCANCODE_LCTRL)) {
						App->scene->selected_obj.clear();
						App->scene->selected_obj.push_back(picked_obj);
					}
					else {
						if ((*App->scene->selected_obj.begin())->is_UI == !picked_obj->is_UI) {
							app_log->AddLog("Cannot select UI GameObject and scene GameObject at the same time!");
						}
						else {
							App->scene->selected_obj.push_back(picked_obj);
						}
					}
				}
				else
					App->scene->selected_obj.clear();
			}
		}

		window_pos = ImGui::GetItemRectMin();
		window_size = ImGui::GetWindowSize();

		if (&camera == App->camera->game_camera)
			game_window_pos = window_pos;
		if (&camera == App->camera->editor_camera)
		{
			if (!App->scene->selected_obj.empty() && !App->scene->selected_obj.front()->isStatic()) // Not draw guizmo if it is static
			{
				ImGuizmo::SetDrawlist();
				App->gui->DrawGuizmo(window_pos, window_size);
			}
			
			App->camera->enable_camera_control = ImGui::IsItemHovered();
		}

		ImGui::End();
	}
	else 
		camera.initFrameBuffer();

	if (&camera == App->camera->editor_camera)
		if (!App->scene->selected_obj.empty() && !App->scene->selected_obj.front()->isStatic() && !App->scene->selected_obj.front()->is_UI)
			App->gui->DrawGizmoMenuTab();

}

void ModuleUI::DrawColorPickerWindow(const char* label, Color* color, bool* closing_bool, Color* reference_col)
{
	ImGui::Begin(label, closing_bool);
	ImGui::Text("Use right click to change color picker mode");
	ImGui::ColorPicker4(label, &color->r, 0, &reference_col->r);
	ImGui::End();
}

void ModuleUI::DrawScriptEditor()
{
	if (App->scripting->edited_scripts.find(open_script_path) != App->scripting->edited_scripts.end())
		App->scripting->edited_scripts.at(open_script_path) = script_editor.GetText();
	else
		App->scripting->edited_scripts.insert(std::make_pair(open_script_path, script_editor.GetText()));
	
	disable_keyboard_control = true; // Will disable keybord control forever
	ImGui::PushFont(ui_fonts[IMGUI_DEFAULT]);
	auto cpos = script_editor.GetCursorPosition();
	ImGui::Begin("Script Editor", &open_tabs[SCRIPT_EDITOR], ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
	ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save"))
			{
				auto textToSave = script_editor.GetText();
				App->fs.SetFileString(open_script_path.c_str(), textToSave.c_str());
			}
			if (ImGui::MenuItem("Quit", "Alt-F4")) {
				// Exit or something
			}
	
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			bool ro = script_editor.IsReadOnly();
			if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
				script_editor.SetReadOnly(ro);
			ImGui::Separator();

			if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && script_editor.CanUndo()))
				script_editor.Undo();
			if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && script_editor.CanRedo()))
				script_editor.Redo();

			ImGui::Separator();

			if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, script_editor.HasSelection()))
				script_editor.Copy();
			if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && script_editor.HasSelection()))
				script_editor.Cut();
			if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && script_editor.HasSelection()))
				script_editor.Delete();
			if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
				script_editor.Paste();

			ImGui::Separator();

			if (ImGui::MenuItem("Select all", nullptr, nullptr))
				script_editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(script_editor.GetTotalLines(), 0));

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Dark palette"))
				script_editor.SetPalette(TextEditor::GetDarkPalette());
			if (ImGui::MenuItem("Light palette"))
				script_editor.SetPalette(TextEditor::GetLightPalette());
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, script_editor.GetTotalLines(),
		script_editor.IsOverwrite() ? "Ovr" : "Ins",
		script_editor.CanUndo() ? "*" : " ",
		script_editor.GetLanguageDefinition().mName.c_str(), open_script_path.c_str());

	TextEditor::CommandKeys c_keys;
	c_keys.ctrl = (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT);
	c_keys._X = App->input->GetKey(SDL_SCANCODE_X) == KEY_DOWN;
	c_keys._Y = App->input->GetKey(SDL_SCANCODE_Y) == KEY_DOWN;
	c_keys._Z = App->input->GetKey(SDL_SCANCODE_Z) == KEY_DOWN;
	c_keys._C = App->input->GetKey(SDL_SCANCODE_C) == KEY_DOWN;
	c_keys._V = App->input->GetKey(SDL_SCANCODE_V) == KEY_DOWN;

	script_editor.Render("TextEditor", ui_fonts[IMGUI_DEFAULT], c_keys);
	ImGui::PopFont();
	ImGui::End();
}

void ModuleUI::DrawBuildMenu()
{
	ImGui::Begin("Make Build", &open_tabs[BUILD_MENU]);

	ImGui::Text("Select a path to make the build:");
	ImGui::PushID("setPath");
	static std::string buildPath;
	char* buffer = new char[buildPath.size()];
	memcpy(buffer, buildPath.c_str(), sizeof(char)*buildPath.size());
	ImGui::InputTextEx("", buffer, buildPath.size(), { 0,0 }, ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);
	RELEASE_ARRAY(buffer);
	ImGui::PopID();
	ImGui::SameLine();
	if (ImGui::RadioButton("Click to choose path", false))
	{
		buildPath = openFileWID(true);
	}
	ImGui::NewLine();

	ImGui::Text("Write Build Name:");
	ImGui::PushID("name");
	static char buildName[15];
	ImGui::InputTextEx("", buildName, 15, {125, 23}, ImGuiInputTextFlags_None);
	ImGui::PopID();
	ImGui::NewLine();

	ImGui::Text("Select Scenes:");
	ImGui::BeginChild("scenes", { 0,100 }, true);
	ImGui::Text("Main  |  Scenes");
	int i = 0;
	bool remove = false;
	int removeIndex = 0;
	for (std::list<resource_deff>::iterator it_s = build_scenes.begin(); it_s != build_scenes.end(); ++it_s)
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 3);
		bool tick = main_scene[i];
		ImGui::PushID(i);
		if (ImGui::Checkbox("", &tick))
		{
			for (int j = 0; j < main_scene.size(); ++j)
			{
				main_scene[j] = false;
			}
			main_scene[i] = true;
		}
		ImGui::PopID();
		main_scene[i] = (main_scene[i] && !tick) ? true : tick;
		ImGui::SameLine(57);

		ImGui::Text((*it_s).asset.c_str());

		ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - 50);
		ImGui::PushID("Remove" + i);
		if (ImGui::Button("Remove"))
		{
			remove = true;
			removeIndex = i;
		}
		ImGui::PopID();

		i++;
	}
	if (remove)
	{
		build_scenes.erase(std::next(build_scenes.begin(), removeIndex));
		bool wasMain = main_scene[removeIndex];
		main_scene.erase(std::next(main_scene.begin(), removeIndex));
		if (wasMain && main_scene.size() > 0)
			main_scene[0] = true;
		remove = false;
	}
	static bool pickScene = false;
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 3);
	if (ImGui::Button("+", { 23,23 }))
	{
		pickScene = true;
	}
	if (pickScene)
	{
		std::list<resource_deff> scene_res;
		App->resources->getSceneResourceList(scene_res, build_scenes);

		ImGui::Begin("Scene selector", &pickScene);
		for (auto it = scene_res.begin(); it != scene_res.end(); it++) {
			resource_deff scene_deff = (*it);
			if (ImGui::MenuItem(scene_deff.asset.c_str())) {
				build_scenes.push_back((*it));
				main_scene.push_back(build_scenes.size() == 1);
				pickScene = false;
				break;
			}
		}
		ImGui::End();
	}
	ImGui::EndChild();

	static bool building = false;
	static bool errorBuilding = false;
	static bool includeReleaseFile = true;
	ImGui::Checkbox("Include Release File", &includeReleaseFile);
	if (building)
	{
		building = false;
		errorBuilding = !App->exporter->CreateBuild(buildPath.c_str(), buildName, includeReleaseFile);
		if (!errorBuilding)
			open_tabs[BUILD_MENU] = false;
	}
	ImGui::NewLine();
	ImGui::SetCursorPosX((ImGui::GetWindowContentRegionWidth() / 2) - 35);
	if (ImGui::Button("Create Build") && buildPath != "" && strlen(buildName) > 0)
	{
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionWidth() / 2) - 20);
		ImGui::Text("Creating...");
		building = true;
		errorBuilding = false;
	}
	if (errorBuilding)
	{
		ImGui::NewLine();
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionWidth() / 2) - 60);
		ImGui::TextColored({ 1, 0, 0, 1 }, "Error: No release file!");
	}
	ImGui::End();
}

uint ModuleUI::getMainScene() const
{
	uint index = 0;
	uint ret = 0;

	for (int i = 0; i < main_scene.size(); ++i)
	{
		if (main_scene[i])
		{
			index = i;
			break;
		}
	}
	if (build_scenes.size() > 0)
	{
		ret = (*std::next(build_scenes.begin(), index)).uuid;
	}

	return ret;
}

void ModuleUI::DrawGizmoMenuTab() {

	ImGui::Begin("Toolbar##Gizmo toolbar", nullptr);

	if (App->camera->editor_camera->getFrustum()->type == math::FrustumType::OrthographicFrustum)
		ImGui::TextColored(ImVec4(1.5f, 1.0f, 0.0f, 1.0), "WARNING: ImGuizmo is not compatible with orthographic camera");
	else
	{

		if (ImGui::ImageButton((void*)ui_textures[GUIZMO_SELECT]->getGLid(), ImVec2(32, 32), nullptr, ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, !draw_guizmo ? 1.0f : 0.0f)) || (App->input->GetKey(SDL_SCANCODE_Q) == KEY_DOWN && App->input->GetMouseButton(SDL_BUTTON_RIGHT) != KEY_REPEAT))
			draw_guizmo = !draw_guizmo;


		ImGui::SameLine();
		if (ImGui::ImageButton((void*)ui_textures[GUIZMO_TRANSLATE]->getGLid(), ImVec2(32, 32), nullptr, ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, gizmo_operation == ImGuizmo::OPERATION::TRANSLATE && draw_guizmo ? 1.0f : 0.0f)) || (App->input->GetKey(SDL_SCANCODE_W) == KEY_DOWN && App->input->GetMouseButton(SDL_BUTTON_RIGHT) != KEY_REPEAT)) {
			gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;
			draw_guizmo = true;
		}

		ImGui::SameLine();
		if (ImGui::ImageButton((void*)ui_textures[GUIZMO_ROTATE]->getGLid(), ImVec2(32, 32), nullptr, ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, gizmo_operation == ImGuizmo::OPERATION::ROTATE && draw_guizmo ? 1.0f : 0.0f)) || (App->input->GetKey(SDL_SCANCODE_E) == KEY_DOWN && App->input->GetMouseButton(SDL_BUTTON_RIGHT) != KEY_REPEAT)) {
			gizmo_operation = ImGuizmo::OPERATION::ROTATE;
			draw_guizmo = true;
		}

		ImGui::SameLine();
		if (ImGui::ImageButton((void*)ui_textures[GUIZMO_SCALE]->getGLid(), ImVec2(32, 32), nullptr, ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, gizmo_operation == ImGuizmo::OPERATION::SCALE&& draw_guizmo ? 1.0f : 0.0f)) || (App->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN && App->input->GetMouseButton(SDL_BUTTON_RIGHT) != KEY_REPEAT)) {
			gizmo_operation = ImGuizmo::OPERATION::SCALE;
			draw_guizmo = true;
		}

		ImGui::SameLine();
		if (ImGui::ImageButton((void*)ui_textures[GUIZMO_LOCAL]->getGLid(), ImVec2(32, 32), nullptr, ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, gizmo_mode == ImGuizmo::MODE::LOCAL && draw_guizmo ? 1.0f : 0.0f)))
		{
			gizmo_mode = ImGuizmo::MODE::LOCAL;
			draw_guizmo = true;
		}

		ImGui::SameLine();
		if (ImGui::ImageButton((void*)ui_textures[GUIZMO_GLOBAL]->getGLid(), ImVec2(32, 32), nullptr, ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, gizmo_mode == ImGuizmo::MODE::WORLD && draw_guizmo ? 1.0f : 0.0f)))
		{
			gizmo_mode = ImGuizmo::MODE::WORLD;
			draw_guizmo = true;
		}
	}

	ImGui::End();

}

void ModuleUI::DrawGuizmo(ImVec2 window_pos, ImVec2 window_size)
{
	if (draw_guizmo)
	{

		ImGuizmo::SetRect(window_pos.x, window_pos.y, window_size.x, window_size.y);

		
		float3 guizmoPos = float3::zero;
		float3 guizmoScale = float3::zero;
		Quat guizmoRot = Quat::identity;

		//-------------guizmoUI (2d)
		if ((*App->scene->selected_obj.begin())->is_UI) {
			for (auto it = App->scene->selected_obj.begin(); it != App->scene->selected_obj.end(); it++) { //sets the pos of the guizmo UI in average of their positions
				ComponentRectTransform* transform = (ComponentRectTransform*)(*it)->getComponent(RECTTRANSFORM);
				guizmoPos += float3(transform->getAnchor().x, transform->getAnchor().y, 0);
				guizmoScale += float3(transform->getWidth(), transform->getHeight(), 0);
			}
		}
		//------------guizmo (3d)

		else {
			for (auto it = App->scene->selected_obj.begin(); it != App->scene->selected_obj.end(); it++) { //sets the pos of the guizmo in average of their positions
				ComponentTransform* transform = (ComponentTransform*)(*it)->getComponent(TRANSFORM);
				Transform* trans = transform->global;
				guizmoPos += trans->getPosition();
				guizmoScale += trans->getScale();
				if(App->scene->selected_obj.size() == 1)
					guizmoRot = trans->getRotation();
			}
		}

		guizmoPos /= App->scene->selected_obj.size();
		guizmoScale /= App->scene->selected_obj.size();

		Transform aux_transform;
		switch (gizmo_operation)
		{
		case ImGuizmo::OPERATION::TRANSLATE:
			aux_transform.setRotation(guizmoRot);
			aux_transform.setPosition(guizmoPos); break;
		case ImGuizmo::OPERATION::ROTATE:
			aux_transform.setPosition(guizmoPos);
			aux_transform.setRotation(guizmoRot); break;
		case ImGuizmo::OPERATION::SCALE:
			aux_transform.setPosition(guizmoPos);
			aux_transform.setRotation(guizmoRot);
			aux_transform.setScale(guizmoScale); break;
		default:
			break;
		}
		
		aux_transform.CalculateMatrix();
		float4x4 mat = float4x4(aux_transform.getMatrix());

		float4x4 projection4x4 = (App->camera->current_camera->getFrustum()->ProjectionMatrix());
		float4x4 view4x4 = App->camera->current_camera->getFrustum()->ViewMatrix();
		view4x4.Transpose();

		mat.Transpose();
		ImGuizmo::Manipulate((float*)view4x4.v, (float*)projection4x4.v, gizmo_operation, gizmo_mode, (float*)mat.v);
		mat.Transpose();
		if (ImGuizmo::IsUsing())
		{
			float3 new_pos = float3::zero;
			float3 new_rot = float3::zero;
			float3 new_scale = float3::zero;

			float3 displacement = mat.TranslatePart() - aux_transform.getPosition();
			static const float max_graduated_scale = 20.f;

			for (auto it = App->scene->selected_obj.begin(); it != App->scene->selected_obj.end(); it++) {

				//------- UI
				if ((*App->scene->selected_obj.begin())->is_UI) {
					ComponentRectTransform* selectedPos = (ComponentRectTransform*)(*it)->getComponent(RECTTRANSFORM);
					float2 pos = selectedPos->getLocalPos();
					float2 dim = float2(selectedPos->getWidth(), selectedPos->getHeight());
					float2 anchor = selectedPos->getAnchor();

					switch (gizmo_operation)
					{
					case ImGuizmo::OPERATION::TRANSLATE:
						new_pos.x = pos.x + displacement.x;
						new_pos.y = pos.y + displacement.y;
						new_pos.z = 0;
						if (!Equal(displacement.x, 0.0f) || !Equal(displacement.y, 0.0f))
							using_guizmo = true;
						selectedPos->setPos(float2(new_pos.x, new_pos.y));
						break;

					case ImGuizmo::OPERATION::SCALE:
						new_scale.x = mat.GetScale().x;
						new_scale.y = mat.GetScale().y;
						new_scale.z = 0;

						if (abs(selectedPos->getHeight() - new_scale.y) < max_graduated_scale) { selectedPos->setHeight(new_scale.y); }
						if (abs(selectedPos->getWidth() - new_scale.x) < max_graduated_scale) { selectedPos->setWidth(new_scale.x); }

						app_log->AddLog("%f, %f", new_scale.x, new_scale.y);
						break;
					default:
						break;
					}
				}
				//--------not UI
				else {
					ComponentTransform* selectedTrans = (ComponentTransform*)(*it)->getComponent(TRANSFORM);
					Transform* trans = selectedTrans->global;
					switch (gizmo_operation)
					{
					case ImGuizmo::OPERATION::TRANSLATE:
						new_pos.x = selectedTrans->constraints[0][0] ? trans->getPosition().x : (trans->getPosition().x + displacement.x);
						new_pos.y = selectedTrans->constraints[0][1] ? trans->getPosition().y : (trans->getPosition().y + displacement.y);
						new_pos.z = selectedTrans->constraints[0][2] ? trans->getPosition().z : (trans->getPosition().z + displacement.z);
						if (!Equal(displacement.x, 0.0f) || !Equal(displacement.y, 0.0f) || !Equal(displacement.z, 0.0f))
							using_guizmo = true;
						trans->setPosition(new_pos);
						break;
					case ImGuizmo::OPERATION::ROTATE:
						if (App->scene->selected_obj.size() > 1)
						{
							new_rot.x = selectedTrans->constraints[1][0] ? trans->getRotationEuler().x : mat.RotatePart().ToEulerXYZ().x;
							new_rot.y = selectedTrans->constraints[1][1] ? trans->getRotationEuler().y : mat.RotatePart().ToEulerXYZ().y;
							new_rot.z = selectedTrans->constraints[1][2] ? trans->getRotationEuler().z : mat.RotatePart().ToEulerXYZ().z;
							trans->setRotation(Quat::FromEulerXYZ(new_rot.x, new_rot.y, new_rot.z) * trans->getRotation());
						}
						else
						{
							new_rot.x = selectedTrans->constraints[1][0] ? trans->getRotationEuler().x : mat.RotatePart().ToEulerXYZ().x;
							new_rot.y = selectedTrans->constraints[1][1] ? trans->getRotationEuler().y : mat.RotatePart().ToEulerXYZ().y;
							new_rot.z = selectedTrans->constraints[1][2] ? trans->getRotationEuler().z : mat.RotatePart().ToEulerXYZ().z;
							trans->setRotation(Quat::FromEulerXYZ(new_rot.x, new_rot.y, new_rot.z));
						}
						break;
					case ImGuizmo::OPERATION::SCALE:
						new_scale.x = selectedTrans->constraints[2][0] ? trans->getScale().x : mat.GetScale().x;
						new_scale.y = selectedTrans->constraints[2][1] ? trans->getScale().y : mat.GetScale().y; // changed z to y to set correct the modification
						new_scale.z = selectedTrans->constraints[2][2] ? trans->getScale().z : mat.GetScale().z;

						trans->setScale(new_scale);
						break;
					default:
						break;
					}
					trans->CalculateMatrix();
					selectedTrans->GlobalToLocal();
				}
			}
		}
		else if (!mat.Equals(aux_transform.getMatrix()))
		{
			App->scene->AskAutoSaveScene();
			using_guizmo = false;
		}
	}
}

void ModuleUI::DrawTagSelection(GameObject* object) {

	std::string object_tag = object->tag; // Current tag
	int inx = 0;						  // Index of the current tag


	std::string posible_tags; // All the tags in the same string
	bool inx_found = false; // Stop when tag is found

	for (auto it = App->scripting->tags.begin(); it != App->scripting->tags.end(); it++){
		// Store every tag in the same string
		posible_tags += (*it);
		posible_tags += '\0';

		// Figure out which inx is the tag of the gameobject
		if (object_tag == (*it))
			inx_found = true;
		if (!inx_found) {
			inx++;
		}
	}
	if (ImGui::Combo("Tag selector", &inx, posible_tags.c_str())) {
		// Out of the selected index, extract the "tag" of the gameobject and return it
		int inx_it = 0;
		for (auto it = App->scripting->tags.begin(); it != App->scripting->tags.end(); it++) {
			if (inx_it == inx){
				object->tag = (*it);
				break;
			}
			inx_it++;
		}
	}
	
}

void ModuleUI::SaveConfig(JSON_Object* config) const
{
	json_object_set_boolean(config, "log", open_log_menu);						//NOT DELETE
	
}

void ModuleUI::LoadConfig(const JSON_Object* config) 
{
	open_log_menu				= json_object_get_boolean(config, "log"); //NOT DELETE

	open_tabs[SCRIPT_EDITOR] = false;
}

bool ModuleUI::isMouseOnUI() const
{
	return ImGui::GetIO().WantCaptureMouse;// && !hoveringScene;
}

bool ModuleUI::keepKeyboard() const
{
	return ImGui::GetIO().WantCaptureKeyboard;
}


void ModuleUI::InvisibleDockingBegin() {
	ImGuiWindowFlags window = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	static ImGuiDockNodeFlags optional = ImGuiDockNodeFlags_PassthruDockspace;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Kuroko Engine", &docking_background, window);
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGuiIO& io = ImGui::GetIO();

	ImGuiID dockspace_id = ImGui::GetID("The dockspace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), optional);
}

void ModuleUI::InvisibleDockingEnd() {
	ImGui::End();
}
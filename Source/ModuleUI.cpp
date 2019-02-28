 #include "ModuleUI.h"
#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleCamera3D.h"
#include "ModuleRenderer3D.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleImporter.h"
#include "ModuleAudio.h"
#include "ModuleTimeManager.h"
#include "ModuleResourcesManager.h"
#include "ModuleScripting.h"
#include "Applog.h"
#include "ScriptData.h"

#include "ImGui/imgui_impl_sdl.h"
#include "ImGui/imgui_impl_opengl2.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui.h"

#include "GameObject.h"
#include "ComponentMesh.h"
#include "FontManager.h"
#include "ComponentCanvas.h"
#include "ComponentTextUI.h"
#include "ComponentRectTransform.h"
#include "ComponentTransform.h"
#include "ComponentParticleEmitter.h"
#include "ComponentScript.h"
#include "ComponentAudioSource.h"
#include "ComponentAABB.h"
#include "ComponentCamera.h"
#include "ComponentImageUI.h"
#include "ComponentCheckBoxUI.h"
#include "ComponentButtonUI.h"
#include "ComponentAnimation.h"
#include "ComponentBillboard.h"
#include "Transform.h"
#include "Camera.h"
#include "Quadtree.h"
#include "ResourceTexture.h"
#include "Resource3dObject.h"
#include "ResourceAnimation.h"
#include "Skybox.h"
#include "FileSystem.h"
#include "Include_Wwise.h"


#include "Random.h"
#include "VRAM.h"
#include "WinItemDialog.h" 

#include "Assimp/include/version.h"

#include "glew-2.1.0\include\GL\glew.h"
#include "SDL\include\SDL_opengl.h"
#include <gl/GL.h>
#include <gl/GLU.h>

#include <experimental/filesystem>
#include <fstream>

#pragma comment( lib, "glew-2.1.0/lib/glew32.lib")
#pragma comment( lib, "glew-2.1.0/lib/glew32s.lib")


ModuleUI::ModuleUI(Application* app, bool start_enabled) : Module(app, start_enabled) {
	name = "gui";
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
	io = &ImGui::GetIO();

	ui_textures[PLAY]				= (Texture*)App->importer->ImportTexturePointer("Editor textures/Play.png");
	ui_textures[PAUSE]				= (Texture*)App->importer->ImportTexturePointer("Editor textures/Pause.png");
	ui_textures[STOP]				= (Texture*)App->importer->ImportTexturePointer("Editor textures/Stop.png");
	ui_textures[ADVANCE]			= (Texture*)App->importer->ImportTexturePointer("Editor textures/Advance.png");

	ui_textures[GUIZMO_TRANSLATE]	= (Texture*)App->importer->ImportTexturePointer("Editor textures/translate.png");
	ui_textures[GUIZMO_ROTATE]		= (Texture*)App->importer->ImportTexturePointer("Editor textures/rotate.png");
	ui_textures[GUIZMO_SCALE]		= (Texture*)App->importer->ImportTexturePointer("Editor textures/scale.png");
	ui_textures[GUIZMO_LOCAL]		= (Texture*)App->importer->ImportTexturePointer("Editor textures/Guizmo_local.png");
	ui_textures[GUIZMO_GLOBAL]		= (Texture*)App->importer->ImportTexturePointer("Editor textures/Guizmo_global.png");
	ui_textures[GUIZMO_SELECT]		= (Texture*)App->importer->ImportTexturePointer("Editor textures/Guizmo_select.png");

	ui_textures[NO_TEXTURE]			= (Texture*)App->importer->ImportTexturePointer("Editor textures/no_texture.png");

	ui_textures[FOLDER_ICON]		= (Texture*)App->importer->ImportTexturePointer("Editor textures/folder_icon.png");
	ui_textures[OBJECT_ICON]		= (Texture*)App->importer->ImportTexturePointer("Editor textures/object_icon.png");
	ui_textures[SCENE_ICON]			= (Texture*)App->importer->ImportTexturePointer("Editor textures/scene_icon.png");
	ui_textures[RETURN_ICON]		= (Texture*)App->importer->ImportTexturePointer("Editor textures/return_icon.png");
	ui_textures[SCRIPT_ICON]		= (Texture*)App->importer->ImportTexturePointer("Editor textures/script_icon.png");
	ui_textures[PREFAB_ICON]		= (Texture*)App->importer->ImportTexturePointer("Editor textures/prefab_icon.png");

	ui_textures[CAUTION_ICON]		= (Texture*)App->importer->ImportTexturePointer("Editor textures/caution_icon_32.png");
	ui_textures[WARNING_ICON]		= (Texture*)App->importer->ImportTexturePointer("Editor textures/warning_icon_32.png");

	ui_fonts[TITLES]				= io->Fonts->AddFontFromFileTTF("Fonts/title.ttf", 16.0f);
	ui_fonts[REGULAR]				= io->Fonts->AddFontFromFileTTF("Fonts/regular.ttf", 18.0f);
	ui_fonts[IMGUI_DEFAULT]			= io->Fonts->AddFontDefault();
	//ui_fonts[REGULAR_BOLD]		= io->Fonts->AddFontFromFileTTF("Fonts/regular_bold.ttf", 18.0f);
	//ui_fonts[REGULAR_ITALIC]		= io->Fonts->AddFontFromFileTTF("Fonts/regular_italic.ttf", 18.0f);
	//ui_fonts[REGULAR_BOLDITALIC]	= io->Fonts->AddFontFromFileTTF("Fonts/regular_bold_italic.ttf", 18.0f);
	
	open_tabs[BUILD_MENU] = false;

	io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io->IniFilename = "Settings\\imgui.ini";
	docking_background = true;


	// HARDCODE
	//App->scene->AskSceneLoadFile("Assets/Scenes/animation.scene");

	return true;
}

update_status ModuleUI::PreUpdate(float dt) {

	// Start the ImGui frame
	ImGui_ImplOpenGL2_NewFrame();

	ImGui_ImplSDL2_NewFrame(App->window->main_window->window);
	ImGui::NewFrame();

	

	//ImGui::ShowDemoWindow();
	return UPDATE_CONTINUE;
}

update_status ModuleUI::Update(float dt) {


	InvisibleDockingBegin();
	static bool file_save = false;
	disable_keyboard_control = false;


	if (open_tabs[CONFIGURATION]) 
	{
		ImGui::Begin("Configuration", &open_tabs[CONFIGURATION]);

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

		if (ImGui::Button("Reset Camera"))
			App->camera->editor_camera->Reset();

		ImGui::End();
	}

	if (open_tabs[HIERARCHY])
		DrawHierarchyTab();

	Camera* prev_selected = App->camera->background_camera;
	App->camera->selected_camera = nullptr;

	if (open_tabs[OBJ_INSPECTOR])
		DrawObjectInspectorTab();

	if (!App->camera->selected_camera)
		App->camera->selected_camera = prev_selected;

	if (open_tabs[PRIMITIVE])
		DrawPrimitivesTab();

	if (open_tabs[ABOUT])
		DrawAboutLeaf();
	
	if (open_tabs[LOG])
		app_log->Draw("App log",&open_tabs[LOG]);

	if (open_tabs[TIME_CONTROL])
		DrawTimeControlWindow();

	if (open_tabs[QUADTREE_CONFIG])
		DrawQuadtreeConfigWindow();

	if (open_tabs[CAMERA_MENU])
		DrawCameraMenuWindow();

	if (open_tabs[VIEWPORT_MENU])
		DrawViewportsWindow();

	if (open_tabs[ASSET_WINDOW])
		DrawAssetsWindow();

	if (open_tabs[RESOURCES_TAB]) 
		DrawResourcesWindow();
	
	if (open_tabs[SKYBOX_MENU])
		DrawSkyboxWindow();

	if (open_tabs[SCRIPT_EDITOR])
		DrawScriptEditor();

	if (open_tabs[BUILD_MENU])
		DrawBuildMenu();

	if (!App->scene->selected_obj.empty() && !(*App->scene->selected_obj.begin())->is_static && !(*App->scene->selected_obj.begin())->is_UI) // Not draw guizmo if it is static
		App->gui->DrawGuizmo();

	for (auto it = App->camera->game_cameras.begin(); it != App->camera->game_cameras.end(); it++)
	{
		if ((*it)->draw_in_UI)
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
				else{
					file_save = true;
				}
				
			}
			if (ImGui::MenuItem("Save scene as...")){
				file_save = true;
			}
			if (ImGui::MenuItem("Load scene")){
				std::string file_path = openFileWID();
				App->scene->AskSceneLoadFile((char*)file_path.c_str());
			}
			if(ImGui::BeginMenu("Configuration")){
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
			ImGui::MenuItem("Hierarchy", NULL, &open_tabs[HIERARCHY]);
			ImGui::MenuItem("Object Inspector", NULL, &open_tabs[OBJ_INSPECTOR]);
			ImGui::MenuItem("Primitive", NULL, &open_tabs[PRIMITIVE]);
			ImGui::MenuItem("Configuration", NULL, &open_tabs[CONFIGURATION]);
			ImGui::MenuItem("Log", NULL, &open_tabs[LOG]);
			ImGui::MenuItem("Time control", NULL, &open_tabs[TIME_CONTROL]);
			ImGui::MenuItem("Quadtree", NULL, &open_tabs[QUADTREE_CONFIG]);
			ImGui::MenuItem("Camera Menu", NULL, &open_tabs[CAMERA_MENU]);
			ImGui::MenuItem("Assets", NULL, &open_tabs[ASSET_WINDOW]);
			ImGui::MenuItem("Resources", NULL, &open_tabs[RESOURCES_TAB]);
			ImGui::MenuItem("Skybox", NULL, &open_tabs[SKYBOX_MENU]);
			ImGui::MenuItem("Script Editor", NULL, &open_tabs[SCRIPT_EDITOR]);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {
			ImGui::MenuItem("About", NULL, &open_tabs[ABOUT]);
			if(ImGui::MenuItem("Documentation"))
				App->requestBrowser("https://github.com/Skyway666/Kuroko-Engine/wiki");
			if(ImGui::MenuItem("Download latest"))
				App->requestBrowser("https://github.com/Skyway666/Kuroko-Engine/releases");
			if(ImGui::MenuItem("Report a bug"))
				App->requestBrowser("https://github.com/Skyway666/Kuroko-Engine/issues");
			ImGui::EndMenu();
		}

		std::string current_viewport = "Viewport: Free camera";

		switch (App->camera->selected_camera->getViewportDir())
		{
		case VP_RIGHT: current_viewport = "Viewport: Right"; break;
		case VP_LEFT: current_viewport	= "Viewport: Left"; break;
		case VP_UP: current_viewport	= "Viewport: Up"; break;
		case VP_DOWN: current_viewport	= "Viewport: Down"; break;
		case VP_FRONT: current_viewport = "Viewport: Front"; break;
		case VP_BACK: current_viewport	= "Viewport: Back"; break;
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

			ImGui::MenuItem("Open viewport menu", nullptr, open_tabs[VIEWPORT_MENU]);
			
			ImGui::EndMenu();
		}
	
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

	if (App->input->GetKey(SDL_SCANCODE_V) == KEY_DOWN && !disable_keyboard_control) {
		open_tabs[VIEWPORT_MENU] = !open_tabs[VIEWPORT_MENU];
		for (int i = 0; i < 6; i++)
			App->camera->viewports[i]->active = open_tabs[VIEWPORT_MENU];
	}
	InvisibleDockingEnd();
	return UPDATE_CONTINUE;

}

update_status ModuleUI::PostUpdate(float dt) {
	// Rendering
	ImGui::Render();
	
	glViewport(0, 0, (int)io->DisplaySize.x, (int)io->DisplaySize.y);

	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

	return UPDATE_CONTINUE;

}

bool ModuleUI::CleanUp() 
{	
	for (int i = 0; i < LAST_UI_TEX; i++) { // Cleanup textures
		delete ui_textures[i];
	}
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




void ModuleUI::DrawHierarchyTab()
{
	ImGui::Begin("Hierarchy Tab", &open_tabs[HIERARCHY]);
	ImGui::PushFont(ui_fonts[REGULAR]);

	int id = 0;
	std::list<GameObject*> root_objs;
	App->scene->getRootObjs(root_objs);

	bool item_hovered = false;

	for (auto it = root_objs.begin(); it != root_objs.end(); it++)
		if (DrawHierarchyNode(*(*it), id)) 
			item_hovered = true;
		
	if (ImGui::IsWindowHovered())
	{
		if (!item_hovered&&App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
			App->scene->selected_obj.clear();
		else if(App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_DOWN && !item_hovered)
			ImGui::OpenPopup("##hierarchy context menu");
	}

	if (ImGui::BeginPopup("##hierarchy context menu"))
	{
		if (ImGui::MenuItem("Empty gameobject"))
		{
			GameObject* parent = nullptr;
			if (!App->scene->selected_obj.empty())
				parent = *App->scene->selected_obj.begin();
			
			GameObject* go = new GameObject("Empty", parent);
			if (!App->scene->selected_obj.empty())
				(*App->scene->selected_obj.begin())->addChild(go);
		}
		if (ImGui::TreeNode("UI"))
		{
			if (ImGui::MenuItem("UI_Image"))
			{
				GameObject* parent = nullptr;
				if (!App->scene->selected_obj.empty() ) {
					if ((*App->scene->selected_obj.begin())->getComponent(RECTTRANSFORM) != nullptr) {
						parent = *(App->scene->selected_obj.begin());						
					}

				}
				else {
					parent = App->scene->getCanvasGameObject();// creates or checks for the cnavas					
				}
				if (parent != nullptr) {
					GameObject* image = new GameObject("UI_Image", parent, true);
					image->addComponent(Component_type::UI_IMAGE);
					parent->addChild(image);
				}
			}
			if (ImGui::MenuItem("UI_Text"))
			{
				GameObject* parent = nullptr;
				if (!App->scene->selected_obj.empty()) {
					if ((*App->scene->selected_obj.begin())->getComponent(RECTTRANSFORM) != nullptr) {
						parent = *App->scene->selected_obj.begin();
					}
				}
				else {
					parent = App->scene->getCanvasGameObject();// creates or checks for the cnavas					
				}
				if (parent != nullptr) {
					GameObject* text = new GameObject("UI_Text", parent, true);
					text->addComponent(Component_type::UI_TEXT);
					parent->addChild(text);
				}
				
			}
			
			if (ImGui::MenuItem("UI_Button"))
			{
				GameObject* parent = nullptr;
				if (!App->scene->selected_obj.empty()) {
					if ((*App->scene->selected_obj.begin())->getComponent(RECTTRANSFORM) != nullptr) {
						parent = (*App->scene->selected_obj.begin());
					}
				}
				else {
					parent = App->scene->getCanvasGameObject();// creates or checks for the cnavas					
				}
				if (parent != nullptr) {
					GameObject* button = new GameObject("UI_Button", parent, true);
					button->addComponent(Component_type::UI_IMAGE);
					button->addComponent(Component_type::UI_BUTTON);
					parent->addChild(button);
				}
				
			}
			if (ImGui::MenuItem("UI_CheckBox"))
			{
				GameObject* parent = nullptr;
				if (!App->scene->selected_obj.empty()) {
					if ((*App->scene->selected_obj.begin())->getComponent(RECTTRANSFORM) != nullptr) {
						parent = *App->scene->selected_obj.begin();
					}
				}
				else {
					parent = App->scene->getCanvasGameObject();// creates or checks for the cnavas					
				}
				if (parent != nullptr) {
					GameObject* chbox = new GameObject("UI_CheckBox", parent, true);
					chbox->addComponent(Component_type::UI_IMAGE);
					chbox->addComponent(Component_type::UI_CHECKBOX);
					parent->addChild(chbox);
				}
				
			}
			ImGui::TreePop();
		}
		ImGui::EndPopup();
	}

	ImGui::PopFont();
	ImGui::End();
}

bool ModuleUI::DrawHierarchyNode(GameObject& game_object, int& id) 
{
	id++;
	static int selection_mask = (1 << 2);
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick; 

	std::list<GameObject*> children;
	game_object.getChildren(children);

	if(children.empty())
		node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; 
	for (auto it = App->scene->selected_obj.begin(); it != App->scene->selected_obj.end(); it++) {
		if (*it == &game_object) {
			node_flags |= ImGuiTreeNodeFlags_Selected;
			break;
		}
	}

	bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)game_object.getUUID(), node_flags, game_object.getName().c_str()) && !children.empty();
	bool item_hovered = ImGui::IsItemHovered();

	
	if(!App->scene->selected_obj.empty() && (*App->scene->selected_obj.begin()) == &game_object)
		selection_mask = (1 << id);
	else if (App->scene->selected_obj.empty())
		selection_mask = (1 >> id);


	if (ImGui::IsItemClicked()) {
		if (!App->input->GetKey(SDL_SCANCODE_LCTRL)) {
			App->scene->selected_obj.clear();
		}
		int lastSize = App->scene->selected_obj.size();// checks if already is selected and diselects it
		App->scene->selected_obj.remove(&game_object);
		if(lastSize == App->scene->selected_obj.size())
		{
			App->scene->selected_obj.push_back(&game_object);
		}
		
	}
	else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
		ImGui::OpenPopup(("##" + game_object.getName() + std::to_string(id) + "Object context menu").c_str());

	static int show_rename = -1;
	if (ImGui::BeginPopup(("##" + game_object.getName() + std::to_string(id) + "Object context menu").c_str()))
	{
		if (ImGui::Button(("Duplicate##" + game_object.getName() + std::to_string(id) + "Duplicate gobj button").c_str()))
			App->scene->duplicateGameObject(&game_object);

		if (ImGui::Button(("Rename##" + game_object.getName() + std::to_string(id) + "Rename gobj button").c_str()))
			show_rename = id;

		if (ImGui::Button(("Delete##" + game_object.getName() + std::to_string(id) + "Delete gobj button").c_str()))
			App->scene->deleteGameObjectRecursive(&game_object);

		if (ImGui::Button(("Save to prefab##" + game_object.getName() + std::to_string(id) + "prefab save gobj button").c_str()))
			App->scene->SavePrefab(&game_object, (game_object.getName()).c_str());

		ImGui::EndPopup();
	}

	if (node_open)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 3);

		for (std::list<GameObject*>::iterator it = children.begin(); it != children.end(); it++)
			if (DrawHierarchyNode(*(*it), id))
				item_hovered = true;

		ImGui::PopStyleVar();
		ImGui::TreePop();
	}

	if (show_rename == id)
	{
		static bool rename_open = true;
		disable_keyboard_control = true;
		ImGui::SetNextWindowPos(ImVec2(700, 320), ImGuiCond_FirstUseEver);
		ImGui::Begin("Rename object", &rename_open);
		ImGui::PushFont(ui_fonts[REGULAR]);

		static char rename_buffer[64];
		ImGui::PushItemWidth(ImGui::GetWindowSize().x - 60);
		ImGui::InputText("##Rename to", rename_buffer, 64);

		ImGui::SameLine();
		if (ImGui::Button("OK##Change name"))
		{
			game_object.Rename(rename_buffer);
			show_rename = -1;
		}

		ImGui::PopFont();
		ImGui::End();

		if(!rename_open)
			show_rename = -1;
	}

	return item_hovered;
}

void ModuleUI::DrawObjectInspectorTab()
{
	ImGui::Begin("Object inspector", &open_tabs[OBJ_INSPECTOR]);
	ImGui::PushFont(ui_fonts[REGULAR]);

	static bool select_script = false;
	if (App->scene->selected_obj.size() == 1) {
		GameObject* selected_obj = (*App->scene->selected_obj.begin());

		if (selected_obj)
		{
			ImGui::Text("Name: %s", selected_obj->getName().c_str());

			ImGui::Checkbox("Active", &selected_obj->is_active);
			ImGui::SameLine();
			if (ImGui::Checkbox("Static", &selected_obj->is_static)) // If an object is set/unset static, reload the quadtree
				App->scene->quadtree_reload = true;

			DrawTagSelection(selected_obj);
			// Add a new tag
			static char new_tag[64];
			ImGui::InputText("New Tag", new_tag, 64);
			if (ImGui::Button("Add Tag")) {
				App->scripting->tags.push_back(new_tag);
				for (int i = 0; i < 64; i++)
					new_tag[i] = '\0';

			}

			if (ImGui::CollapsingHeader("Add component"))
			{
				if (ImGui::Button("Add Mesh"))	selected_obj->addComponent(MESH);
				if (ImGui::Button("Add Camera"))  selected_obj->addComponent(CAMERA);
				if (ImGui::Button("Add Script")) select_script = true;
				if (ImGui::Button("Add Animation")) selected_obj->addComponent(ANIMATION);
				if (ImGui::Button("Add Audio Source")) selected_obj->addComponent(AUDIOSOURCE);
				if (ImGui::Button("Add Listener")) selected_obj->addComponent(AUDIOLISTENER); 
				if (ImGui::Button("Add Billboard")) selected_obj->addComponent(BILLBOARD);
				if (ImGui::Button("Add Particle Emitter")) selected_obj->addComponent(PARTICLE_EMITTER);
			}

			std::list<Component*> components;
			selected_obj->getComponents(components);

			std::list<Component*> components_to_erase;
			int id = 0;
			for (std::list<Component*>::iterator it = components.begin(); it != components.end(); it++) {
				if (!DrawComponent(*(*it), id))
					components_to_erase.push_back(*it);
				id++;
			}

			for (std::list<Component*>::iterator it = components_to_erase.begin(); it != components_to_erase.end(); it++)
				selected_obj->removeComponent(*it);

			if (select_script) {
				std::list<resource_deff> script_res;
				App->resources->getScriptResourceList(script_res);

				ImGui::Begin("Script selector", &select_script);
				for (auto it = script_res.begin(); it != script_res.end(); it++) {
					resource_deff script_deff = (*it);
					if (ImGui::MenuItem(script_deff.asset.c_str())) {
						ComponentScript* c_script = (ComponentScript*)selected_obj->addComponent(SCRIPT);
						c_script->assignScriptResource(script_deff.uuid);
						select_script = false;
						break;
					}
				}

				ImGui::End();
			}
		}
	}

	ImGui::PopFont();
	ImGui::End();

}

bool ModuleUI::DrawComponent(Component& component, int id)
{
	bool ret = true;

	ComponentCamera* camera = nullptr; // aux pointer

	std::string tag;
	ImGui::PushID(component.getUUID());

	switch (component.getType())
	{
	case MESH:
		tag = "Mesh##" + std::to_string(id);
		if (ImGui::CollapsingHeader(tag.c_str()))
		{
			ComponentMesh* c_mesh = (ComponentMesh*)&component;
			static bool wireframe_enabled;
			static bool mesh_active;
			static bool draw_normals;

			wireframe_enabled = c_mesh->getWireframe();
			draw_normals = c_mesh->getDrawNormals();
			mesh_active = c_mesh->isActive();

			if (ImGui::Checkbox("Active## mesh_active", &mesh_active))
				c_mesh->setActive(mesh_active);

			if (mesh_active)
			{

				if (ImGui::Checkbox("Wireframe", &wireframe_enabled))
					c_mesh->setWireframe(wireframe_enabled);
				ImGui::SameLine();
				if (ImGui::Checkbox("Draw normals", &draw_normals))
					c_mesh->setDrawNormals(draw_normals);
				
				if (!c_mesh->getMesh()) {
					static bool add_mesh_menu = false;
					if (ImGui::Button("Add mesh")) {
						add_mesh_menu = true;
					}

					if (add_mesh_menu) {

						std::list<resource_deff> mesh_res;
						App->resources->getMeshResourceList(mesh_res);

						ImGui::Begin("Mesh selector", &add_mesh_menu);
						for (auto it = mesh_res.begin(); it != mesh_res.end(); it++) {
							resource_deff mesh_deff = (*it);
							if (ImGui::MenuItem(mesh_deff.asset.c_str())) {
								App->resources->deasignResource(c_mesh->getMeshResource());
								App->resources->assignResource(mesh_deff.uuid);
								c_mesh->setMeshResourceId(mesh_deff.uuid);
								add_mesh_menu = false;
								break;
							}
						}

						ImGui::End();
					}
				}

				if (Mesh* mesh = c_mesh->getMesh())
				{
					if (ImGui::TreeNode("Mesh Options"))
					{
						uint vert_num, poly_count;
						bool has_normals, has_colors, has_texcoords;
						if (ImGui::Button("Remove mesh")) {
							App->resources->deasignResource(c_mesh->getMeshResource());
							c_mesh->setMeshResourceId(0);
						}
						mesh->getData(vert_num, poly_count, has_normals, has_colors, has_texcoords);
						ImGui::Text("vertices: %d, poly count: %d, ", vert_num, poly_count);
						ImGui::Text(has_normals ? "normals: Yes," : "normals: No,");
						ImGui::Text(has_colors ? "colors: Yes," : "colors: No,");
						ImGui::Text(has_texcoords ? "tex coords: Yes" : "tex coords: No");

						ImGui::TreePop();
					}
				}
				
				
				if (ImGui::TreeNode("Material"))
				{
					if (Material* material = c_mesh->getMaterial())
					{
						static int preview_size = 128;
						ImGui::Text("Id: %d", material->getId());
						ImGui::SameLine();
						if (ImGui::Button("remove material"))
						{
							delete c_mesh->getMaterial();
							c_mesh->setMaterial(nullptr);
							ImGui::TreePop();
							return true;
						}

						ImGui::Text("Preview size");
						ImGui::SameLine();
						if (ImGui::Button("64")) preview_size = 64;
						ImGui::SameLine();
						if (ImGui::Button("128")) preview_size = 128;
						ImGui::SameLine();
						if (ImGui::Button("256")) preview_size = 256;

						if (ImGui::TreeNode("diffuse"))
						{
							Texture* texture = nullptr;
							if (ResourceTexture* tex_res = (ResourceTexture*)App->resources->getResource(material->getTextureResource(DIFFUSE)))
								texture = tex_res->texture;


							ImGui::Image(texture ? (void*)texture->getGLid() : (void*)ui_textures[NO_TEXTURE]->getGLid(), ImVec2(preview_size, preview_size));
							ImGui::SameLine();

							int w = 0; int h = 0;
							if (texture)
								texture->getSize(w, h);

							ImGui::Text("texture data: \n x: %d\n y: %d", w, h);

							//if (ImGui::Button("Load checkered##Dif: Load checkered"))
							//	material->setCheckeredTexture(DIFFUSE);
							//ImGui::SameLine()
							if (ImGui::Button("Load(from asset folder)##Dif: Load"))
							{
								std::string texture_path = openFileWID();
								uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
								if (new_resource != 0) {
									App->resources->assignResource(new_resource);
									App->resources->deasignResource(material->getTextureResource(DIFFUSE));
									material->setTextureResource(DIFFUSE, new_resource);
								}
							}
							ImGui::TreePop();
						}

						if (ImGui::TreeNode("ambient (feature not avaliable yet)"))
						{
							//ImGui::Image(material->getTexture(AMBIENT) ? (void*)material->getTexture(AMBIENT)->getGLid() : (void*)ui_textures[NO_TEXTURE]->getGLid(), ImVec2(preview_size, preview_size));

							//if (ImGui::Button("Load checkered##Amb: Load checkered"))
							//	material->setCheckeredTexture(AMBIENT);
							//ImGui::SameLine();
							//if (ImGui::Button("Load##Amb: Load"))
							//{
							//	std::string texture_path = openFileWID();
							//	if (Texture* tex = (Texture*)App->importer->Import(texture_path.c_str(), I_TEXTURE))
							//		c_mesh->getMaterial()->setTexture(AMBIENT, tex);
							//}
							ImGui::TreePop();
						}

						if (ImGui::TreeNode("normals (feature not avaliable yet)"))
						{
							//ImGui::Image(material->getTexture(NORMALS) ? (void*)material->getTexture(NORMALS)->getGLid() : (void*)ui_textures[NO_TEXTURE]->getGLid(), ImVec2(preview_size, preview_size));

							//if (ImGui::Button("Load checkered##Nor: Load checkered"))
							//	material->setCheckeredTexture(NORMALS);
							//ImGui::SameLine();
							//if (ImGui::Button("Load##Nor: Load"))
							//{
							//	std::string texture_path = openFileWID();
							//	if (Texture* tex = (Texture*)App->importer->Import(texture_path.c_str(), I_TEXTURE))
							//		c_mesh->getMaterial()->setTexture(NORMALS, tex);
							//}
							ImGui::TreePop();
						}

						if (ImGui::TreeNode("lightmap (feature not avaliable yet)"))
						{
							//ImGui::Image(material->getTexture(LIGHTMAP) ? (void*)material->getTexture(LIGHTMAP)->getGLid() : (void*)ui_textures[NO_TEXTURE]->getGLid(), ImVec2(preview_size, preview_size));

							//if (ImGui::Button("Load checkered##Lgm: Load checkered"))
							//	material->setCheckeredTexture(LIGHTMAP);
							//ImGui::SameLine();
							//if (ImGui::Button("Load##Lgm: Load"))
							//{
							//	std::string texture_path = openFileWID();
							//	if (Texture* tex = (Texture*)App->importer->Import(texture_path.c_str(), I_TEXTURE))
							//		c_mesh->getMaterial()->setTexture(LIGHTMAP, tex);
							//}
							ImGui::TreePop();
						}
					}
					else
					{
						ImGui::TextWrapped("No material assigned!");

						if (c_mesh->getMesh())
						{
							static bool draw_colorpicker = false;
							static Color reference_color = c_mesh->getMesh()->tint_color;
							static GameObject* last_selected = c_mesh->getParent();

							std::string label = c_mesh->getParent()->getName() + " color picker";

							if (last_selected != c_mesh->getParent())
								reference_color = c_mesh->getMesh()->tint_color;

							ImGui::SameLine();
							if (ImGui::ColorButton((label + "button").c_str(), ImVec4(c_mesh->getMesh()->tint_color.r, c_mesh->getMesh()->tint_color.g, c_mesh->getMesh()->tint_color.b, c_mesh->getMesh()->tint_color.a)))
								draw_colorpicker = !draw_colorpicker;

							if (draw_colorpicker)
								DrawColorPickerWindow(label.c_str(), (Color*)&c_mesh->getMesh()->tint_color, &draw_colorpicker, (Color*)&reference_color);
							else
								reference_color = c_mesh->getMesh()->tint_color;

							last_selected = c_mesh->getParent();
						}

						if (ImGui::Button("Add material"))
						{
							Material* mat = new Material();
							c_mesh->setMaterial(mat);
						}

					}
					ImGui::TreePop();
				}

				if (c_mesh->components_bones.size() > 0 && ImGui::TreeNode("Connected Bones"))
				{
					ImGui::Text("Num Bones: %d", c_mesh->components_bones.size());
					ImGui::TreePop();
				}
			
			}

			if (ImGui::Button("Remove Component##Remove mesh"))
				ret = false;
		}
		break;
	case TRANSFORM:
		if (ImGui::CollapsingHeader("Transform")) 
		{
			ImGui::TextWrapped("Drag the parameters to change them, or ctrl+click on one of them to set it's value");
			ComponentTransform* c_trans = (ComponentTransform*)&component;

			static float3 position;
			static float3 rotation;
			static float3 scale;

			Transform* transform = nullptr;

			if (c_trans->getMode() == LOCAL)
			{
				transform = c_trans->local;
				ImGui::Text("Current mode: Local");
				ImGui::SameLine();
				if (ImGui::Button("Global"))
					c_trans->setMode(GLOBAL);
			}
			else
			{
				transform = c_trans->global;
				ImGui::Text("Current mode: Global");
				ImGui::SameLine();
				if (ImGui::Button("Local"))
					c_trans->setMode(LOCAL);
			}

			position = transform->getPosition();
			rotation = transform->getRotationEuler();
			scale = transform->getScale();

			//position
			ImGui::Text("Position:");
			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if(!c_trans->constraints[0][0])		ImGui::DragFloat("##p x", &position.x, 0.01f, 0.0f, 0.0f, "%.02f");
			else								ImGui::Text("%.2f", position.x);

			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if (!c_trans->constraints[0][1])	ImGui::DragFloat("##p y", &position.y, 0.01f, 0.0f, 0.0f, "%.02f");
			else								ImGui::Text("%.2f", position.y);

			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if (!c_trans->constraints[0][2])	ImGui::DragFloat("##p z", &position.z, 0.01f, 0.0f, 0.0f, "%.02f");
			else								ImGui::Text("%.2f", position.z);
			
			//rotation
			ImGui::Text("Rotation:");
			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if (!c_trans->constraints[1][0])	ImGui::DragFloat("##r x", &rotation.x, 0.2f, -180.0f, 180.0f, "%.02f");
			else								ImGui::Text("%.2f", rotation.z);

			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if (!c_trans->constraints[1][1])	ImGui::DragFloat("##r y", &rotation.y, 0.2f, -180.0f, 180.0f, "%.02f");
			else								ImGui::Text("%.2f", rotation.x);

			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if (!c_trans->constraints[1][2])	ImGui::DragFloat("##r z", &rotation.z, 0.2f, -180.0f, 180.0f, "%.02f");
			else								ImGui::Text("%.2f", rotation.y);

			//scale
			ImGui::Text("   Scale:");
			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if (!c_trans->constraints[2][0])	ImGui::DragFloat("##s x", &scale.x, 0.01f, -1000.0f, 1000.0f, "%.02f");
			else								ImGui::Text("%.2f", scale.x);

			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if (!c_trans->constraints[2][1])	ImGui::DragFloat("##s y", &scale.y, 0.01f, -1000.0f, 1000.0f, "%.02f");
			else								ImGui::Text("%.2f", scale.y);

			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if (!c_trans->constraints[2][2])	ImGui::DragFloat("##s z", &scale.z, 0.01f, -1000.0f, 1000.0f, "%.02f");
			else								ImGui::Text("%.2f", scale.z);

			if (ImGui::Button("Reset Transform"))
			{
				position = float3::zero; rotation = float3::zero, scale = float3::one;
			}

			ImGui::Checkbox("Draw axis", &c_trans->draw_axis);
			
			if (ImGui::CollapsingHeader("	Constraints"))
			{
				ImGui::Text("Position:");
				ImGui::SameLine();
				ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
				ImGui::Checkbox("##constraint p x", &c_trans->constraints[0][0]);

				ImGui::SameLine();
				ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
				ImGui::Checkbox("##constraint p y", &c_trans->constraints[0][1]);

				ImGui::SameLine();
				ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
				ImGui::Checkbox("##constraint p z", &c_trans->constraints[0][2]);


				ImGui::Text("Rotation:");
				ImGui::SameLine();
				ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
				ImGui::Checkbox("##constraint r x", &c_trans->constraints[1][0]);

				ImGui::SameLine();
				ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
				ImGui::Checkbox("##constraint r y", &c_trans->constraints[1][1]);

				ImGui::SameLine();
				ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
				ImGui::Checkbox("##constraint r z", &c_trans->constraints[1][2]);


				ImGui::Text("   Scale:");
				ImGui::SameLine();
				ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
				ImGui::Checkbox("##constraint s x", &c_trans->constraints[2][0]);

				ImGui::SameLine();
				ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
				ImGui::Checkbox("##constraint s y", &c_trans->constraints[2][1]);

				ImGui::SameLine();
				ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
				ImGui::Checkbox("##constraint s z", &c_trans->constraints[2][2]);
			}

			transform->setPosition(position);
			transform->setRotationEuler(rotation);
			transform->setScale(scale);
		}
		break;

	case C_AABB:
		if (ImGui::CollapsingHeader("AABB"))
		{
			ComponentAABB* aabb = (ComponentAABB*)&component;

			static bool aabb_active;
			aabb_active = aabb->isActive();

			if (ImGui::Checkbox("Active##active AABB", &aabb_active))
				aabb->setActive(aabb_active);

			if (aabb_active)
			{
				static bool aabb_drawn;
				aabb_drawn = aabb->draw_aabb;

				if (ImGui::Checkbox("draw AABB", &aabb_drawn))
					aabb->draw_aabb = aabb_drawn;

				static bool obb_drawn;
				obb_drawn = aabb->draw_obb;

				ImGui::SameLine();
				if (ImGui::Checkbox("draw OBB", &obb_drawn))
					aabb->draw_obb = obb_drawn;

				if (ImGui::Button("Reload##Reload AABB"))
					aabb->Reload();
			}

		}
		break;
	case CAMERA:

		camera = (ComponentCamera*)&component;

		if (ImGui::CollapsingHeader("Camera"))
		{
			static bool camera_active;
			camera_active = camera->isActive();

			if (ImGui::Checkbox("Active##active camera", &camera_active))
				camera->setActive(camera_active);

			ImGui::Checkbox("Draw camera view", &camera->getCamera()->draw_in_UI);

			ImGui::Checkbox("Draw frustum", &camera->getCamera()->draw_frustum);

			ImGui::Checkbox("Draw depth", &camera->getCamera()->draw_depth);

			static bool overriding;
			overriding = (camera->getCamera() == App->camera->override_editor_cam_culling);
			if (ImGui::Checkbox("Override Frustum Culling", &overriding))
			{
				if (!overriding)	App->camera->override_editor_cam_culling = nullptr;
				else				App->camera->override_editor_cam_culling = camera->getCamera();
			}

			if (camera_active)
			{
				static float3 offset;
				offset = camera->offset;
				ImGui::Text("Offset:");
				ImGui::SameLine();
				ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
				ImGui::DragFloat("##o x", &offset.x, 0.01f, -1000.0f, 1000.0f, "%.02f");

				ImGui::SameLine();
				ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
				ImGui::DragFloat("##o y", &offset.y, 0.01f, -1000.0f, 1000.0f, "%.02f");

				ImGui::SameLine();
				ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
				ImGui::DragFloat("##o z", &offset.z, 0.01f, -1000.0f, 1000.0f, "%.02f");

				camera->offset = offset;
			}

			if (ImGui::Button("Remove##Remove camera"))
				ret = false;
		}
		break;
	case SCRIPT:
	{
		ComponentScript* c_script = (ComponentScript*)&component;
		std::string component_title = c_script->script_name + "(Script)";
		if (ImGui::CollapsingHeader(component_title.c_str())) {

			if (!c_script->instance_data) {
				ImGui::Image((void*)ui_textures[WARNING_ICON]->getGLid(), ImVec2(16, 16));
				ImGui::SameLine();
				ImGui::Text("Compile error");
			}
			else{
				for (auto it = c_script->instance_data->vars.begin(); it != c_script->instance_data->vars.end(); it++) {
				
					if (!(*it).isPublic())
						continue;

					ImportedVariable* curr = &(*it);
					std::string unique_tag = "##" + curr->getName();

					static int type = 0;

					if (!curr->isTypeForced())
					{
						type = curr->getType() - 1;
						if (ImGui::Combo(unique_tag.c_str(), &type, "Bool\0String\0Numeral\0"))
						{
							curr->setType((ImportedVariable::WrenDataType)(type + 1));
							Var nuller;
							switch (curr->getType())
							{
							case ImportedVariable::WrenDataType::WREN_BOOL:
								nuller.value_bool = false;
								break;
							case ImportedVariable::WrenDataType::WREN_NUMBER:
								nuller.value_number = 0;
								break;
							case ImportedVariable::WrenDataType::WREN_STRING:
								curr->value_string = "";
								break;
							}
							curr->SetValue(nuller);
							curr->setEdited(true);
						}
					}

					ImGui::Text(curr->getName().c_str());
					ImGui::SameLine();

					static char buf[200] = "";
					Var variable = curr->GetValue();

					switch (curr->getType()) {
					case ImportedVariable::WREN_NUMBER:
						if (ImGui::InputFloat((unique_tag + " float").c_str(), &variable.value_number))
						{
							curr->SetValue(variable);
							curr->setEdited(true);
						}
						break;
					case ImportedVariable::WREN_STRING:
					{
						strcpy(buf, curr->value_string.c_str());

						if (ImGui::InputText((unique_tag + " string").c_str(), buf, sizeof(buf)))
						{
							curr->value_string = buf;
							curr->SetValue(variable);
							curr->setEdited(true);
						}
					}
						break;
					case ImportedVariable::WREN_BOOL:
						if (ImGui::Checkbox((unique_tag + " bool").c_str(), &variable.value_bool))
						{
							curr->SetValue(variable);
							curr->setEdited(true);
						}
						break;
					}

				}
			}

			if(ResourceScript* res_script = (ResourceScript*)App->resources->getResource(c_script->getResourceUUID())){ // Check if resource exists
				if (ImGui::Button("Edit script")) {
					open_tabs[SCRIPT_EDITOR] = true;
					open_script_path = res_script->asset;

						if (App->scripting->edited_scripts.find(open_script_path) != App->scripting->edited_scripts.end())
							script_editor.SetText(App->scripting->edited_scripts.at(open_script_path));
						else {
							std::ifstream t(open_script_path.c_str());
							if (t.good()) {
								std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
									script_editor.SetText(str);
							}
						}
				}
			}
			if (ImGui::Button("Remove##Remove script"))
				ret = false;

		}
	
	}
	break;
	case AUDIOLISTENER:
		if (ImGui::CollapsingHeader("Audio Listener"))
		{
			if (ImGui::Checkbox("Mute", &App->audio->muted))
			{
				App->audio->SetVolume(App->audio->volume);
			}
			if (ImGui::SliderInt("Volume", &App->audio->volume, 0, 100))
			{
				App->audio->muted = false;
				App->audio->SetVolume(App->audio->volume);
			}
			if (ImGui::Button("Remove##Remove audioListener"))
				ret = false;
		}
		break;

	case AUDIOSOURCE:
		if (ImGui::CollapsingHeader("Audio Source"))
		{
			static bool select_audio = false;
			
			AkUniqueID ID = ((ComponentAudioSource*)&component)->sound_ID;
			if (ID != 0)
			{
				ImGui::TextColored({ 0, 1, 0, 1 }, ((ComponentAudioSource*)&component)->name.c_str());

				if (ImGui::Button("Play"))
				{
					((ComponentAudioSource*)&component)->sound_go->PlayEvent(ID);
				}
				ImGui::SameLine();
				if (ImGui::Button("Stop"))
				{
					((ComponentAudioSource*)&component)->sound_go->StopEvent(ID);
				}
				ImGui::SameLine();
				if (ImGui::Button("Pause"))
				{
					((ComponentAudioSource*)&component)->sound_go->PauseEvent(ID);
				}
				ImGui::SameLine();
				if (ImGui::Button("Resume"))
				{
					((ComponentAudioSource*)&component)->sound_go->ResumeEvent(ID);
				}
				if (ImGui::SliderInt("Volume", &((ComponentAudioSource*)&component)->volume, 0, 100))
				{
					App->audio->SetVolume(((ComponentAudioSource*)&component)->volume, ((ComponentAudioSource*)&component)->sound_go->GetID());
				}
				if (ImGui::SliderInt("Pitch", &((ComponentAudioSource*)&component)->pitch, -100, 100))
				{
					App->audio->SetPitch(((ComponentAudioSource*)&component)->pitch, ((ComponentAudioSource*)&component)->sound_go->GetID());
				}
				if (ImGui::Button("Change Audio Event")) select_audio = true;
			}
			else
			{
				ImGui::TextColored({ 1, 0, 0, 1 }, "No Audio Event assigned!");
				if (ImGui::Button("Set Audio Event")) select_audio = true;
			}

			if (select_audio)
			{
				ImGui::Begin("Select Audio Event", &select_audio);
				for (auto it = App->audio->events.begin(); it != App->audio->events.end(); it++) {
					
					if (ImGui::MenuItem((*it).c_str())) {
						((ComponentAudioSource*)&component)->SetSoundID(AK::SoundEngine::GetIDFromString((*it).c_str()));
						((ComponentAudioSource*)&component)->SetSoundName((*it).c_str());
						select_audio = false;
						break;
					}
				}
				ImGui::End();
			}

			if (ImGui::Button("Remove##Remove audioSource"))
				ret = false;
		}
		break;
	case CANVAS:
		if (ImGui::CollapsingHeader("Canvas"))
		{
			ComponentCanvas* canvas = (ComponentCanvas*)&component;
			ImGui::Text("Resolution  %.0f x %.0f", canvas->getResolution().x, canvas->getResolution().y);
			ImGui::Checkbox("Draw cross hair", &canvas->draw_cross);
		}
		break;
	case RECTTRANSFORM:
		if (ImGui::CollapsingHeader("Rect Transform"))
		{
			ComponentRectTransform* rectTrans = (ComponentRectTransform*)&component;

			static float2 position;
			static float width;
			static float height;

			position = rectTrans->getLocalPos();
			width = rectTrans->getWidth();
			height = rectTrans->getHeight();

			//position
			ImGui::Text("Position:");
			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if (ImGui::DragFloat2("##p", (float*)&position, 0.01f)) { rectTrans->setPos(position); }
			//Width
			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if (ImGui::DragFloat("##h", &width, 0.01f, 0.0f, 0.0f, "%.02f")) { rectTrans->setWidth(width); }
			//Height
			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if (ImGui::DragFloat("##w", &height, 0.01f, 0.0f, 0.0f, "%.02f")) {
				rectTrans->setHeight(height);
			}




			ImGui::Checkbox("Debug draw", &rectTrans->debug_draw);
		}
		break;
	case UI_IMAGE:
		if (ImGui::CollapsingHeader("UI Image"))
		{
			ComponentImageUI* image = (ComponentImageUI*)&component;

			ImGui::Image(image->getResourceTexture() != nullptr ? (void*)image->getResourceTexture()->texture->getGLid() : (void*)ui_textures[NO_TEXTURE]->getGLid(), ImVec2(128, 128));
			ImGui::SameLine();

			int w = 0; int h = 0;
			if (image->getResourceTexture() != nullptr) {
				image->getResourceTexture()->texture->getSize(w, h);
			}

			ImGui::Text("texture data: \n x: %d\n y: %d", w, h);

			ImGui::SliderFloat("Alpha", &image->alpha, 0.0f, 1.0f);

			if (ImGui::Button("Load(from asset folder)##Dif: Load"))
			{
				std::string texture_path = openFileWID();
				uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
				if (new_resource != 0) {
					App->resources->assignResource(new_resource);
					if (image->getResourceTexture() != nullptr)
						App->resources->deasignResource(image->getResourceTexture()->uuid);
					image->setResourceTexture((ResourceTexture*)App->resources->getResource(new_resource));
				}
			}
		}
		break;
	case UI_CHECKBOX:
		if (ImGui::CollapsingHeader("UI CheckBox"))
		{
			ComponentCheckBoxUI* chBox = (ComponentCheckBoxUI*)&component;

			ImGui::Image(chBox->getResourceTexture(CH_IDLE) != nullptr ? (void*)chBox->getResourceTexture(CH_IDLE)->texture->getGLid() : (void*)ui_textures[NO_TEXTURE]->getGLid(), ImVec2(128, 128));
			ImGui::SameLine();

			int w = 0; int h = 0;
			if (chBox->getResourceTexture(CH_IDLE) != nullptr) {
				chBox->getResourceTexture(CH_IDLE)->texture->getSize(w, h);
			}

			ImGui::Text("Idle texture data: \n x: %d\n y: %d", w, h);

			if (ImGui::Button("Load(from asset folder)##Dif: Load"))
			{
				std::string texture_path = openFileWID();
				uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
				if (new_resource != 0) {
					App->resources->assignResource(new_resource);
					if (chBox->getResourceTexture(CH_IDLE) != nullptr)
						App->resources->deasignResource(chBox->getResourceTexture(CH_IDLE)->uuid);
					chBox->setResourceTexture((ResourceTexture*)App->resources->getResource(new_resource), CH_IDLE);
				}
			}
			ImGui::Image(chBox->getResourceTexture(CH_PRESSED) != nullptr ? (void*)chBox->getResourceTexture(CH_PRESSED)->texture->getGLid() : (void*)ui_textures[NO_TEXTURE]->getGLid(), ImVec2(128, 128));
			ImGui::SameLine();

			int w2 = 0; int h2 = 0;
			if (chBox->getResourceTexture(CH_PRESSED) != nullptr) {
				chBox->getResourceTexture(CH_PRESSED)->texture->getSize(w2, h2);
			}

			ImGui::Text("Pressed texture data: \n x: %d\n y: %d", w2, h2);

			if (ImGui::Button("Load(from asset folder)##Dif: Load2"))
			{
				std::string texture_path = openFileWID();
				uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
				if (new_resource != 0) {
					App->resources->assignResource(new_resource);
					if (chBox->getResourceTexture(CH_PRESSED) != nullptr)
						App->resources->deasignResource(chBox->getResourceTexture(CH_PRESSED)->uuid);
					chBox->setResourceTexture((ResourceTexture*)App->resources->getResource(new_resource), CH_PRESSED);
				}
			} // For debug
			bool pressed = chBox->isPressed();
			ImGui::Checkbox("Pressed", &pressed);
			if (pressed != chBox->isPressed()) { chBox->Press(); }
		}
		break;
	case UI_TEXT:
		if (ImGui::CollapsingHeader("UI Text"))
		{
			ComponentTextUI* text = (ComponentTextUI*)&component;

			static const int maxSize = 32;
			if (ImGui::InputText("Label Text", (char*)text->label.text.c_str(), maxSize)) {
				text->SetText(text->label.text.c_str());
			}
			if (ImGui::SliderFloat("Scale", &(text->label.font->scale), 8, MAX_CHARS, "%0.f")) {
				text->SetFontScale(text->label.font->scale);
			}
			ImGui::Checkbox("Draw Characters Frame", &text->drawCharPanel);
			ImGui::Checkbox("Draw Label Frame", &text->drawLabelrect);
			std::string currentFont = text->label.font->fontSrc;
			if (ImGui::BeginCombo("Fonts", currentFont.c_str()))
			{
				std::vector<std::string> fonts = App->fontManager->singleFonts;

				for (int i = 0; i < fonts.size(); i++)
				{
					bool isSelected = false;

					if (strcmp(currentFont.c_str(), fonts[i].c_str()) == 0) {
						isSelected = true;
					}

					if (ImGui::Selectable(fonts[i].c_str(), isSelected)) {
						std::string newFontName = std::string(fonts[i].c_str());
						std::string newFontExtension = std::string(fonts[i].c_str());
						App->fs.getFileNameFromPath(newFontName);
						App->fs.getExtension(newFontExtension);
						newFontName += newFontExtension;
						text->SetFont(newFontName.c_str());

						if (isSelected) {
							ImGui::SetItemDefaultFocus();
						}

					}

				}
				ImGui::EndCombo();

			}
			ImGui::Spacing();
			ImGui::ColorPicker3("Color##2f", (float*)&text->label.color);
		}
		break;
	case UI_BUTTON:
		if (ImGui::CollapsingHeader("UI Button"))
		{
			ComponentButtonUI* button = (ComponentButtonUI*)&component;
			ButtonState state;//debug
			ImGui::Image(button->getResourceTexture(B_IDLE) != nullptr ? (void*)button->getResourceTexture(B_IDLE)->texture->getGLid() : (void*)ui_textures[NO_TEXTURE]->getGLid(), ImVec2(128, 128));
			ImGui::SameLine();

			int w = 0; int h = 0;
			if (button->getResourceTexture(B_IDLE) != nullptr) {
				button->getResourceTexture(B_IDLE)->texture->getSize(w, h);
			}

			ImGui::Text("Idle texture data: \n x: %d\n y: %d", w, h);

			if (ImGui::Button("Load(from asset folder)##Dif: Load"))
			{
				std::string texture_path = openFileWID();
				uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
				if (new_resource != 0) {
					App->resources->assignResource(new_resource);
					if (button->getResourceTexture(B_IDLE) != nullptr)
						App->resources->deasignResource(button->getResourceTexture(B_IDLE)->uuid);
					button->setResourceTexture((ResourceTexture*)App->resources->getResource(new_resource), B_IDLE);
				}
			}

			ImGui::Image(button->getResourceTexture(B_MOUSEOVER) != nullptr ? (void*)button->getResourceTexture(B_MOUSEOVER)->texture->getGLid() : (void*)ui_textures[NO_TEXTURE]->getGLid(), ImVec2(128, 128));
			ImGui::SameLine();

			int w3 = 0; int h3 = 0;
			if (button->getResourceTexture(B_MOUSEOVER) != nullptr) {
				button->getResourceTexture(B_MOUSEOVER)->texture->getSize(w3, h3);
			}

			ImGui::Text("Hover texture data: \n x: %d\n y: %d", w3, h3);

			if (ImGui::Button("Load(from asset folder)##Dif: Load2"))
			{
				std::string texture_path = openFileWID();
				uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
				if (new_resource != 0) {
					App->resources->assignResource(new_resource);
					if (button->getResourceTexture(B_MOUSEOVER) != nullptr)
						App->resources->deasignResource(button->getResourceTexture(B_MOUSEOVER)->uuid);
					button->setResourceTexture((ResourceTexture*)App->resources->getResource(new_resource), B_MOUSEOVER);
				}
			}


			ImGui::Image(button->getResourceTexture(B_PRESSED) != nullptr ? (void*)button->getResourceTexture(B_PRESSED)->texture->getGLid() : (void*)ui_textures[NO_TEXTURE]->getGLid(), ImVec2(128, 128));
			ImGui::SameLine();

			int w2 = 0; int h2 = 0;
			if (button->getResourceTexture(B_PRESSED) != nullptr) {
				button->getResourceTexture(B_PRESSED)->texture->getSize(w2, h2);
			}

			ImGui::Text("Pressed texture data: \n x: %d\n y: %d", w2, h2);

			if (ImGui::Button("Load(from asset folder)##Dif: Load3"))
			{
				std::string texture_path = openFileWID();
				uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
				if (new_resource != 0) {
					App->resources->assignResource(new_resource);
					if (button->getResourceTexture(B_PRESSED) != nullptr)
						App->resources->deasignResource(button->getResourceTexture(B_PRESSED)->uuid);
					button->setResourceTexture((ResourceTexture*)App->resources->getResource(new_resource), B_PRESSED);
				}
			}
			// For debug
			bool idle = false;
			bool hover = false;
			bool pressed = false;
			ImGui::Separator();
			if (ImGui::Button("FadeIn")) {
				button->doFadeIn();
			}
			ImGui::SameLine();
			if (ImGui::Button("FadeOut")) {
				button->doFadeOut();
			}
			if (ImGui::Button("Idle")) { button->setState(B_IDLE); } ImGui::SameLine();
			if (ImGui::Button("Hover")) { button->setState(B_MOUSEOVER); }ImGui::SameLine();
			if (ImGui::Button("Pressed")) { button->setState(B_PRESSED); }
		}
		break;
	case ANIMATION:
		if (ImGui::CollapsingHeader("Animation"))
		{
			//SKELETAL_TODO: missing resource info

			ComponentAnimation* anim = (ComponentAnimation*)&component;
			ResourceAnimation* R_anim = (ResourceAnimation*)App->resources->getResource(anim->getAnimationResource());
			ImGui::Text("Resource: %s", (R_anim!=nullptr)? R_anim->asset.c_str() : "None");

			static bool set_animation_menu = false;
			if (ImGui::Button((R_anim != nullptr)? "Change Animation":"Add Animation")) {
				set_animation_menu = true;
			}

			if (set_animation_menu) {

				std::list<resource_deff> anim_res;
				App->resources->getAnimationResourceList(anim_res);

				ImGui::Begin("Animation selector", &set_animation_menu);
				for (auto it = anim_res.begin(); it != anim_res.end(); it++) {
					resource_deff anim_deff = (*it);
					if (ImGui::MenuItem(anim_deff.asset.c_str())) {
						App->resources->deasignResource(anim->getAnimationResource());
						App->resources->assignResource(anim_deff.uuid);
						anim->setAnimationResource(anim_deff.uuid);
						set_animation_menu = false;
						break;
					}
				}

				ImGui::End();
			}

			static bool animation_active;
			animation_active = anim->isActive();

			if (ImGui::Checkbox("Active##active animation", &animation_active))
				anim->setActive(animation_active);

			ImGui::Checkbox("Loop", &anim->loop);

			ImGui::InputFloat("Speed", &anim->speed, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);

			if (R_anim != nullptr)
			{
				if (App->time->getGameState() != GameState::PLAYING)
				{
					ImGui::Text("Play");
					ImGui::SameLine();
					ImGui::Text("Pause");
				}
				else if (anim->isPaused())
				{
					if (ImGui::Button("Play"))
						anim->Play();
					ImGui::SameLine();
					ImGui::Text("Pause");
				}
				else
				{
					ImGui::Text("Play");
					ImGui::SameLine();
					if (ImGui::Button("Pause"))
						anim->Pause();
				}

				ImGui::Text("Animation info:");
				ImGui::Text(" Duration: %.1f ms", R_anim->getDuration()*1000);
				ImGui::Text(" Animation Bones: %d", R_anim->numBones);
			}

			if (ImGui::Button("Remove Component##Remove animation"))
				ret = false;
		}
		break;
	case BONE:
		if (ImGui::CollapsingHeader("Bone"))
		{

		}
		break;

	case BILLBOARD:
	{
		ComponentBillboard * c_billboard = (ComponentBillboard*)&component;
		if (ImGui::CollapsingHeader("Billboard"))
		{
			if (Material* material = c_billboard->billboard->getMaterial())
			{
				static int preview_size = 128;
				ImGui::Text("Id: %d", material->getId());
				ImGui::SameLine();
				if (ImGui::Button("remove material"))
				{
					delete c_billboard->billboard->getMaterial();
					c_billboard->billboard->setMaterial(nullptr);
					ImGui::TreePop();
					return true;
				}

				ImGui::Text("Preview size");
				ImGui::SameLine();
				if (ImGui::Button("64")) preview_size = 64;
				ImGui::SameLine();
				if (ImGui::Button("128")) preview_size = 128;
				ImGui::SameLine();
				if (ImGui::Button("256")) preview_size = 256;

				Texture* texture = nullptr;
				if (ResourceTexture* tex_res = (ResourceTexture*)App->resources->getResource(material->getTextureResource(DIFFUSE)))
					texture = tex_res->texture;


				ImGui::Image(texture ? (void*)texture->getGLid() : (void*)ui_textures[NO_TEXTURE]->getGLid(), ImVec2(preview_size, preview_size));
				ImGui::SameLine();

				int w = 0; int h = 0;
				if (texture)
					texture->getSize(w, h);

				ImGui::Text("texture data: \n x: %d\n y: %d", w, h);

				//if (ImGui::Button("Load checkered##Dif: Load checkered"))
				//	material->setCheckeredTexture(DIFFUSE);
				//ImGui::SameLine()
			}
			else
				ImGui::TextWrapped("No material assigned");

			if (ImGui::Button("Load material(from asset folder)##Billboard: Load"))
			{
				std::string texture_path = openFileWID();
				uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
				if (new_resource != 0) {
					App->resources->assignResource(new_resource);

					if (Material* material = c_billboard->billboard->getMaterial())
						App->resources->deasignResource(material->getTextureResource(DIFFUSE));
					else
						c_billboard->billboard->setMaterial(new Material());

					c_billboard->billboard->getMaterial()->setTextureResource(DIFFUSE, new_resource);
				}
			}

			ImGui::Checkbox("Use Color", &c_billboard->billboard->useColor);

			if (c_billboard->billboard->useColor)
			{
				static bool draw_colorpicker = false;
				static Color reference_color = c_billboard->billboard->color;
				static GameObject* last_selected = c_billboard->getParent();

				std::string label = c_billboard->getParent()->getName() + " color picker";

				if (last_selected != c_billboard->getParent())
					reference_color = c_billboard->billboard->color;

				ImGui::SameLine();
				if (ImGui::ColorButton((label + "button").c_str(), ImVec4(c_billboard->billboard->color.r, c_billboard->billboard->color.g, c_billboard->billboard->color.b, c_billboard->billboard->color.a)))
					draw_colorpicker = !draw_colorpicker;

				if (draw_colorpicker)
					DrawColorPickerWindow(label.c_str(), (Color*)&c_billboard->billboard->color, &draw_colorpicker, (Color*)&reference_color);
				else
					reference_color = c_billboard->billboard->color;

				last_selected = c_billboard->getParent();
			}

			if (ImGui::CollapsingHeader("Alignement"))
			{
				if (ImGui::Selectable("Screen aligned", c_billboard->billboard->alignment == SCREEN_ALIGN))
					c_billboard->billboard->alignment = SCREEN_ALIGN;

				if (ImGui::Selectable("World aligned", c_billboard->billboard->alignment == WORLD_ALIGN))
					c_billboard->billboard->alignment = WORLD_ALIGN;

				if (ImGui::Selectable("Axis aligned", c_billboard->billboard->alignment == AXIAL_ALIGN))
					c_billboard->billboard->alignment = AXIAL_ALIGN;
			}

			if (ImGui::Button("Remove##Remove billboard"))
				return false;
		}
		break;
	}
	case PARTICLE_EMITTER:
	{
		ComponentParticleEmitter* c_emitter = (ComponentParticleEmitter*)&component;
		if (ImGui::CollapsingHeader("Particle emitter"))
		{
			//Emitter Lifetime
			ImGui::Text("Set to -1 for infinite lifetime");
			if (ImGui::SliderFloat("LifeTime", &c_emitter->emitterLifetime, -1, 100))
				c_emitter->time = 0;

			if (ImGui::Button("Reset"))
				c_emitter->time = 0;

			ImGui::Text("LifeTime: %.2f", c_emitter->emitterLifetime - c_emitter->time);

			ImGui::NewLine();
			int particles = c_emitter->maxParticles;

			ImGui::Text("Set to 0 for uncapped particles");
			if (ImGui::SliderInt("Max particles", &particles, 0, 1000))
				c_emitter->maxParticles = particles;


			ImGui::SliderFloat("Period", &c_emitter->period, MINSPAWNRATE, 10);


			//Area of spawn

			if (ImGui::CollapsingHeader("Spawn Area"))
			{

				if (ImGui::Selectable("AABB", c_emitter->area.type == AAB))
				{
					c_emitter->area.type = AAB;
				}
				if (ImGui::Selectable("Sphere", c_emitter->area.type == SPHERE))
				{
					c_emitter->area.type = SPHERE;
				}
				if (ImGui::Selectable("Point", c_emitter->area.type == AREA_NONE))
				{
					c_emitter->area.type = AREA_NONE;
				}


				switch (c_emitter->area.type)
				{
				case SPHERE:

					ImGui::DragFloat("Radius", &c_emitter->area.sphere.r, 0.1f);
					break;
				case AAB:
				{
					float3 size = c_emitter->area.aabb.Size();
					if (ImGui::DragFloat3("Size", (float*)&size, 0.1f))
					{
						c_emitter->area.aabb.SetFromCenterAndSize(c_emitter->area.aabb.CenterPoint(), size);
					}
				}
				break;
				case AREA_NONE:
				default:
					break;
				}

				ImGui::Separator();
			}

			if (ImGui::CollapsingHeader("Particle"))
			{

				//Direction
				ImGui::DragFloat3("Direction", (float*)&c_emitter->direction, 0.1f);
				ImGui::SliderFloat("Direction Variation", &c_emitter->dirVartiation, 0, 180);
				ImGui::DragFloat3("Gravity", (float*)&c_emitter->gravity, 0.1f);


				//LifeTime
				float minlife = c_emitter->particleLifetime.min;
				float maxlife = c_emitter->particleLifetime.max;

				ImGui::PushID("LT");

				ImGui::Text("Particle Life Time");
				ImGui::SliderFloat("Min", &c_emitter->particleLifetime.min, 0, c_emitter->particleLifetime.max);
				ImGui::SliderFloat("Max", &c_emitter->particleLifetime.max, c_emitter->particleLifetime.min, 100);


				ImGui::PopID();

				//Speed
				ImGui::PushID("Speed Variation");

				ImGui::Text("Speed");
				ImGui::SliderFloat("Min", &c_emitter->speed.min, 0, c_emitter->speed.max);
				ImGui::SliderFloat("Max", &c_emitter->speed.max, c_emitter->speed.min, 100);

				ImGui::PopID();

				//Start Size
				ImGui::PushID("SSize");

				ImGui::Text("Start Size");
				ImGui::SliderFloat("Min", &c_emitter->startSize.min, 0, c_emitter->startSize.max);
				ImGui::SliderFloat("Max", &c_emitter->startSize.max, c_emitter->startSize.min, 100);

				ImGui::PopID();

				//End Size
				ImGui::PushID("ESize");

				ImGui::Text("End Size");
				ImGui::SliderFloat("Min", &c_emitter->endSize.min, 0, c_emitter->endSize.max);
				ImGui::SliderFloat("Max", &c_emitter->endSize.max, c_emitter->endSize.min, 100);

				ImGui::PopID();

				////Start Spin
				//ImGui::PushID("SSpin");

				//ImGui::Text("Start Spin");
				//ImGui::SliderFloat("Min", &startSpin.min, 0, startSpin.max);
				//ImGui::SliderFloat("Max", &startSpin.max, startSpin.min, 100);

				//ImGui::PopID();

				////End Spin
				//ImGui::PushID("ESpin");

				//ImGui::Text("End Spin");
				//ImGui::SliderFloat("Min", &endSpin.min, 0, endSpin.max);
				//ImGui::SliderFloat("Max", &endSpin.max, endSpin.min, 100);

				//ImGui::PopID();

				//Start Color
				ImGui::PushID("SColor");

				ImGui::Text("StartColor");

				ImGui::ColorEdit4("Min", (float*)&c_emitter->startColor.min);
				ImGui::ColorEdit4("Max", (float*)&c_emitter->startColor.max);

				ImGui::PopID();

				//End Color
				ImGui::PushID("EColor");

				ImGui::Text("EndColor");

				ImGui::ColorEdit4("Min", (float*)&c_emitter->endColor.min);
				ImGui::ColorEdit4("Max", (float*)&c_emitter->endColor.max);

				ImGui::PopID();
			}



			if (ImGui::CollapsingHeader("Billboard"))
			{
				if (Material* material = c_emitter->billboard->getMaterial())
				{
					static int preview_size = 128;
					ImGui::Text("Id: %d", material->getId());
					ImGui::SameLine();
					if (ImGui::Button("remove material"))
					{
						delete c_emitter->billboard->getMaterial();
						c_emitter->billboard->setMaterial(nullptr);
						ImGui::TreePop();
						return true;
					}

					ImGui::Text("Preview size");
					ImGui::SameLine();
					if (ImGui::Button("64")) preview_size = 64;
					ImGui::SameLine();
					if (ImGui::Button("128")) preview_size = 128;
					ImGui::SameLine();
					if (ImGui::Button("256")) preview_size = 256;

					Texture* texture = nullptr;
					if (ResourceTexture* tex_res = (ResourceTexture*)App->resources->getResource(material->getTextureResource(DIFFUSE)))
						texture = tex_res->texture;


					ImGui::Image(texture ? (void*)texture->getGLid() : (void*)ui_textures[NO_TEXTURE]->getGLid(), ImVec2(preview_size, preview_size));
					ImGui::SameLine();

					int w = 0; int h = 0;
					if (texture)
						texture->getSize(w, h);

					ImGui::Text("texture data: \n x: %d\n y: %d", w, h);

					//if (ImGui::Button("Load checkered##Dif: Load checkered"))
					//	material->setCheckeredTexture(DIFFUSE);
					//ImGui::SameLine()
				}
				else
					ImGui::TextWrapped("No material assigned");

				if (ImGui::Button("Load material(from asset folder)##Billboard: Load"))
				{
					std::string texture_path = openFileWID();
					uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
					if (new_resource != 0) {
						App->resources->assignResource(new_resource);

						if (Material* material = c_emitter->billboard->getMaterial())
							App->resources->deasignResource(material->getTextureResource(DIFFUSE));
						else
							c_emitter->billboard->setMaterial(new Material());

						c_emitter->billboard->getMaterial()->setTextureResource(DIFFUSE, new_resource);
					}
				}

			}

			if (ImGui::Button("Remove##Remove particle emitter"))
				return false;
		}
		break;
	}
	
	default:
		break;
	}
	ImGui::PopID();

	return ret;
}

void ModuleUI::DrawCameraViewWindow(Camera& camera)
{
	if (FrameBuffer* frame_buffer = camera.getFrameBuffer())
	{
		std::string window_name;

		if (camera.getParent())
			window_name = camera.getParent()->getParent()->getName() + " Camera";
		else
			window_name = camera.getViewportDirString();

		ImGui::Begin(window_name.c_str(), &camera.draw_in_UI, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);

		if(ImGui::IsWindowFocused())
			App->camera->selected_camera = &camera;
		
		ImGui::SetWindowSize(ImVec2(frame_buffer->size_x / 3 + 30 , frame_buffer->size_y / 3 + 40));

		if (ImGui::ImageButton((void*) (camera.draw_depth ? frame_buffer->depth_tex->gl_id : frame_buffer->tex->gl_id), ImVec2(frame_buffer->size_x / 3, frame_buffer->size_y / 3), nullptr, ImVec2(0, 1), ImVec2(1, 0)))
		{
			float x = App->input->GetMouseX(); float y = App->input->GetMouseY();
			ImVec2 window_pos = ImGui::GetItemRectMin();
			x = (((x - window_pos.x) / ImGui::GetItemRectSize().x) * 2)  - 1;
			y = (((y - window_pos.y) / ImGui::GetItemRectSize().y) * -2) + 1;

			App->scene->selected_obj.push_back(App->scene->MousePicking(camera.getParent() ? camera.getParent()->getParent() : nullptr));
		}
		ImGui::End();
	}
	else camera.initFrameBuffer();
}

void ModuleUI::DrawViewportsWindow()
{
	ImGui::Begin("Viewports", nullptr);

	if (ImGui::Button("Close##Close viewports"))
	{
		for (int i = 0; i < 6; i++)
			App->camera->viewports[i]->active = false;

		open_tabs[VIEWPORT_MENU] = false;
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
			for (int i = 0; i < 6; i++)
				App->camera->viewports[i]->active = false;

			App->camera->background_camera->active = false;
			App->camera->background_camera = App->camera->selected_camera = App->camera->viewports[i];
			App->camera->background_camera->active = true;
			open_tabs[VIEWPORT_MENU] = false;
		}
	}
	ImGui::End();
}

void ModuleUI::DrawCameraMenuWindow()
{
	ImGui::Begin("Camera Menu", &open_tabs[CAMERA_MENU]);

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
		}

		if (ImGui::TreeNode(name.c_str()))
		{
			if ((*it)->active)  ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Active");
			else				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Unactive");

			ImGui::SameLine();
			if (*it != App->camera->background_camera)
			{
				static bool is_overriding;
				is_overriding = ((*it) == App->camera->override_editor_cam_culling);
				if (ImGui::Checkbox("Override Frustum Culling", &is_overriding))
				{
					if (!is_overriding)  App->camera->override_editor_cam_culling = nullptr;
					else				 App->camera->override_editor_cam_culling = *it;
				}
			}
			else
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Background camera");
			
			ImGui::Checkbox("Draw camera view", &(*it)->draw_in_UI);

			ImGui::Checkbox("Draw frustum", &(*it)->draw_frustum);

			ImGui::Checkbox("Draw depth", &(*it)->draw_depth);

			if(ImGui::CollapsingHeader("Frustum"))
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

void ModuleUI::DrawAssetsWindow()
{
	ImGui::Begin("Assets Window", &open_tabs[ASSET_WINDOW]);
	int element_size = 64;
	std::string path, name, extension;
	
	int column_num = (int)trunc(ImGui::GetWindowSize().x / (element_size + 20));
	static bool item_hovered = false;

	if (column_num != 0)
	{
		int count = 0;
		int iteration = 0;

		if (ImGui::ImageButton((void*)ui_textures[RETURN_ICON]->getGLid(), ImVec2(24, 17)))
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
			if(extension != ".meta")
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

			if(column_num > 1)
				ImGui::SetColumnWidth(ImGui::GetColumnIndex(), element_size + 20);

			bool draw_caution = false;
			bool draw_warning = false;
			std::string error_message;

			if (it.status().type() == std::experimental::filesystem::v1::file_type::directory)
			{

				if (ImGui::IsMouseDoubleClicked(0))
				{
					ImGui::ImageButton((void*)ui_textures[FOLDER_ICON]->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f));
					if (ImGui::IsItemHovered())
						asset_window_path = it.path().generic_string();
				}
				else {
					if (ImGui::ImageButton((void*)ui_textures[FOLDER_ICON]->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f)))
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
						ImGui::ImageButton((void*)ui_textures[OBJECT_ICON]->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f));
						if (ImGui::IsItemHovered())
							App->resources->Load3dObjectToScene(it.path().generic_string().c_str());
					}
					else{
						if (ImGui::ImageButton((void*)ui_textures[OBJECT_ICON]->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f)))
							selected_asset = it.path().generic_string();
						else if (ImGui::IsItemHovered())
							item_hovered = true;
					}
				}
				else if (type == "texture")
				{
					ResourceTexture* res_tex = (ResourceTexture*)App->resources->getResource(App->resources->getResourceUuid(it.path().generic_string().c_str()));
					if(res_tex){
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
						ImGui::ImageButton((void*)ui_textures[PREFAB_ICON]->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f));
						if (ImGui::IsItemHovered())
							App->scene->AskPrefabLoadFile((char*)it.path().generic_string().c_str(), float3(0,0,0), float3(0, 0, 0));
					}
					else {
						if (ImGui::ImageButton((void*)ui_textures[PREFAB_ICON]->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f)))
							selected_asset = it.path().generic_string();
						else if (ImGui::IsItemHovered())
							item_hovered = true;
					}
				}

				else if(type == "scene")
				{
					if (ImGui::IsMouseDoubleClicked(0)) {
						ImGui::ImageButton((void*)ui_textures[SCENE_ICON]->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f));
						if (ImGui::IsItemHovered())
							App->scene->AskSceneLoadFile((char*)it.path().generic_string().c_str());
					}
					else {
						if (ImGui::ImageButton((void*)ui_textures[SCENE_ICON]->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f)))
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
						ImGui::ImageButton((void*)ui_textures[SCRIPT_ICON]->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f));
						if (ImGui::IsItemHovered())
						{
							open_script_path = it.path().generic_string();
							open_tabs[SCRIPT_EDITOR] = true;

							if (App->scripting->edited_scripts.find(open_script_path) != App->scripting->edited_scripts.end())
								script_editor.SetText(App->scripting->edited_scripts.at(open_script_path));
							else
							{
								std::ifstream t(open_script_path.c_str());
								if (t.good())
								{
									std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
									script_editor.SetText(str);
								}
							}
						}
					}
					else {
						if (ImGui::ImageButton((void*)ui_textures[SCRIPT_ICON]->getGLid(), ImVec2(element_size, element_size), it.path().generic_string().c_str(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, selected_asset == it.path().generic_string() ? 1.0f : 0.0f)))
							selected_asset = it.path().generic_string();
						else if (ImGui::IsItemHovered())
							item_hovered = true;
					}
				}
			}
			
			if (draw_warning || draw_caution)
			{
				ImGui::Image((void*)ui_textures[draw_warning ? WARNING_ICON : CAUTION_ICON]->getGLid(), ImVec2(16, 16));

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
		disable_keyboard_control = true;
		ImGui::Begin("Script Name", &name_script);
		ImGui::PushFont(ui_fonts[REGULAR]);

		static char rename_buffer[64];
		ImGui::InputText("Create as...", rename_buffer, 64);
		ImGui::SameLine();
		if (ImGui::Button("Create")) {
			std::string script_name = rename_buffer;
			std::string full_path = asset_window_path + "/" + script_name + ".wren";
			App->fs.CreateEmptyFile(full_path.c_str());
			open_script_path = full_path;
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
			script_editor.SetText(file_initial_text);
			App->fs.SetFileString(open_script_path.c_str(), file_initial_text.c_str());
			open_tabs[SCRIPT_EDITOR] = true;
			for (int i = 0; i < 64; i++)
				rename_buffer[i] = '\0';
			name_script = false;
		}
		ImGui::PopFont();
		ImGui::End();
	}

	DrawAssetInspector();
}

void ModuleUI::DrawAssetInspector()
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
		if(type == "3dobject")
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
			if(res_tex){
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
						if(type != 0)
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

void ModuleUI::DrawResourcesWindow()
{
	ImGui::Begin("Resources Window", &open_tabs[RESOURCES_TAB]);
	static float refresh_ratio = 1;
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
	if (ImGui::InputFloat("Refresh ratio (seconds)", &refresh_ratio)) {
		App->resources->setRefreshRatio(refresh_ratio * 1000);
	}
	ImGui::End();
}

//void ModuleUI::DrawAudioTab()
//{
//	ImGui::Begin("Audio", &open_tabs[AUDIO]);
//	ImGui::PushFont(ui_fonts[REGULAR]);
//	ImGui::Text("Use this tab to check and play loaded audio files");
//	
//	for (auto it = App->audio->audio_files.begin(); it != App->audio->audio_files.end(); it++)
//	{
//		if (ImGui::TreeNode((*it)->name.c_str()))
//		{
//			ImGui::Text("type: %s", (*it)->type == FX ? "FX" : "Music");
//			ImGui::SameLine();
//			if (ImGui::ImageButton((void*)ui_textures[PLAY]->getGLid(), ImVec2(16, 16)))
//				(*it)->Play();
//			ImGui::SameLine();
//			if (ImGui::ImageButton((void*)ui_textures[STOP]->getGLid(), ImVec2(16, 16)))
//				(*it)->Stop();
//			ImGui::TreePop();
//		}
//	}
//
//  ImGui::PopFont();
//	ImGui::End();
//}

void ModuleUI::DrawPrimitivesTab() 
{
	ImGui::Begin("Primitives", &open_tabs[PRIMITIVE]);
	ImGui::PushFont(ui_fonts[REGULAR]);

	if (ImGui::Button("Add cube")){
		GameObject* cube = new GameObject("Cube");
		cube->addComponent(C_AABB);
		cube->addComponent(new ComponentMesh(cube, Primitive_Cube));
	}
	ImGui::SameLine();
	if (ImGui::Button("Add plane")){
		GameObject* plane = new GameObject("Plane");
		plane->addComponent(C_AABB);
		plane->addComponent(new ComponentMesh(plane, Primitive_Plane));
	}
	if (ImGui::Button("Add sphere")) {
		GameObject* sphere = new GameObject("Sphere");
		sphere->addComponent(C_AABB);
		sphere->addComponent(new ComponentMesh(sphere, Primitive_Sphere));
	}
	ImGui::SameLine();
	if (ImGui::Button("Add cylinder")) {
		GameObject* cylinder = new GameObject("Cylinder");
		cylinder->addComponent(C_AABB);
		cylinder->addComponent(new ComponentMesh(cylinder, Primitive_Cylinder));
	}

	ImGui::PopFont();
	ImGui::End();
}

void ModuleUI::DrawSkyboxWindow()
{
	ImGui::Begin("Skybox", &open_tabs[SKYBOX_MENU]);

	if (Skybox* skybox = App->scene->skybox)
	{
		ImGui::Checkbox("active##skybox active", &skybox->active);

		static float new_distance = skybox->distance;
		ImGui::PushItemWidth(ImGui::GetWindowWidth() - 10);
		if (ImGui::SliderFloat("##skybox distance", &new_distance, 1.0f, 5000.0f, "Distance: %.0f"))
			skybox->setDistance(new_distance);

		ImGui::Checkbox("color mode##skybox color mode", &skybox->color_mode);

		if (skybox->color_mode)
		{
			static bool draw_colorpicker = false;
			static Color reference_color = skybox->color;

				ImGui::SameLine();
			if (ImGui::ColorButton("##skybox_color", ImVec4(skybox->color.r, skybox->color.g, skybox->color.b, skybox->color.a)))
				draw_colorpicker = !draw_colorpicker;

			if (draw_colorpicker)
				DrawColorPickerWindow("Skybox color picker", (Color*)&skybox->color, &draw_colorpicker, (Color*)&reference_color);
			else
				reference_color = skybox->color;
		}
		else
		{
			if (ImGui::Button("Clear all textures##clear all skybox textures"))
			{
				for(int i = 0; i < 6; i++)
					skybox->removeTexture((Direction)i);
			}

			for(int i = 0; i < 6; i++)
			{ 
				char* label = "";
				Texture* texture = nullptr;

				switch (i)
				{
				case LEFT:	label = "left_texture";		texture = skybox->getTexture(LEFT);		break;
				case RIGHT: label = "right_texture";	texture = skybox->getTexture(RIGHT);	break;
				case UP:	label = "up_texture";		texture = skybox->getTexture(UP);		break;
				case DOWN:	label = "down_texture";		texture = skybox->getTexture(DOWN);		break;
				case FRONT: label = "front_texture";	texture = skybox->getTexture(FRONT);	break;
				case BACK:	label = "back_texture";		texture = skybox->getTexture(BACK);		break;
				}

				if (ImGui::TreeNode(label))
				{
					ImGui::Image(texture ? (void*)texture->getGLid() : (void*)ui_textures[NO_TEXTURE]->getGLid(), ImVec2(64, 64));
					ImGui::SameLine();

					int w = 0; int h = 0;
					if (texture)
						texture->getSize(w, h);

					ImGui::Text("texture data: \n x: %d\n y: %d", w, h);

					if (ImGui::Button("Clear texture##clear skybox texture"))
						skybox->removeTexture((Direction)i);

					//if (ImGui::Button("Load checkered##Dif: Load checkered"))
					//	material->setCheckeredTexture(DIFFUSE);
					////ImGui::SameLine()
					if (ImGui::Button("Load(from asset folder)##Dif: Load"))
					{
						std::string texture_path = openFileWID();
						skybox->setTexture((Texture*)App->importer->ImportTexturePointer(texture_path.c_str()), (Direction)i);
					}
					ImGui::TreePop();
				}
			}
		}
	}

	ImGui::End();
	
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

	ImGui::End();
}

void ModuleUI::DrawAboutLeaf()
{
	ImGui::Begin("About", &open_tabs[ABOUT]); 
	ImGui::PushFont(ui_fonts[REGULAR]);

	ImGui::Text("Kuroko Engine");
	ImGui::Separator();
	ImGui::Text("An engine to make videogames");
	ImGui::Text("By Rodrigo de Pedro Lombao and Lucas Garcia Mateu.");
	ImGui::Text("Kuroko Engine is licensed under the MIT License.\n");
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
	if(ImGui::Button("Learn more##imguizmo"))
		App->requestBrowser("https://github.com/CedricGuillemet/ImGuizmo");
	ImGui::Text("ImGuiColorTextEdit");
	ImGui::SameLine();
	if (ImGui::Button("Learn more##texteditor"))
		App->requestBrowser("https://github.com/BalazsJako/ImGuiColorTextEdit");
	ImGui::Text("Wren");
	ImGui::SameLine();
	if (ImGui::Button("Learn more##wren"))
		App->requestBrowser("http://wren.io/");
	ImGui::PopFont();
	ImGui::End();
}

void ModuleUI::DrawGraphicsLeaf() const {
	//starting values
	ImGui::PushFont(ui_fonts[REGULAR]);

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
	ImGui::PopFont();
}

void ModuleUI::DrawWindowConfigLeaf() const
{
	ImGui::PushFont(ui_fonts[REGULAR]);

	Window* window = App->window->main_window;
	if(ImGui::SliderFloat("##Brightness", &window->brightness, 0, 1.0f, "Brightness: %.2f"))
		App->window->setBrightness(window->brightness);

	bool width_mod, height_mod = false;
	width_mod = ImGui::SliderInt("##Window width", &window->width, MIN_WINDOW_WIDTH, MAX_WINDOW_WIDTH, "Width: %d");
	height_mod = ImGui::SliderInt("###Window height", &window->height, MIN_WINDOW_HEIGHT, MAX_WINDOW_HEIGHT, "Height: %d");
	
	if(width_mod || height_mod)
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

	ImGui::PopFont();
}

void ModuleUI::DrawHardwareLeaf() const 
{
	ImGui::PushFont(ui_fonts[REGULAR]);
	
	//CPUs
	ImGui::Text("CPUs");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0, 255, 0, 255),"%i", SDL_GetCPUCount());

	// RAM
	ImGui::Text("System RAM");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0, 255, 0, 255), "%i Gb", SDL_GetSystemRAM());

	// Caps
	ImGui::Text("Caps:");
	ImGui::SameLine();
	if(SDL_HasRDTSC())
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
		
	ImGui::PopFont();
}

void ModuleUI::DrawApplicationLeaf() const 
{
	// HARDCODED (?)
	ImGui::PushFont(ui_fonts[REGULAR]);
	
	ImGui::Text("App name: Kuroko Engine");
	ImGui::Text("Organization: UPC CITM");
	char title[25];
	sprintf_s(title, 25, "Framerate %.1f", App->fps_log[App->fps_log.size() - 1]);
	ImGui::PlotHistogram("##framerate", &App->fps_log[0], App->fps_log.size(), 0, title, 0.0f, 100.0f, ImVec2(310, 100));
	sprintf_s(title, 25, "Milliseconds %.1f", App->ms_log[App->ms_log.size() - 1]);
	ImGui::PlotHistogram("##milliseconds", &App->ms_log[0], App->ms_log.size(), 0, title, 0.0f, 100.0f, ImVec2(310, 100));
	
	ImGui::PopFont();
}

void ModuleUI::DrawEditorPreferencesLeaf() const {

	static float camera_speed = 2.5f;
	if (ImGui::InputFloat("Camera speed", &camera_speed))
		App->camera->camera_speed = camera_speed;


	static float camera_rotation_speed = 0.25f;
	if (ImGui::InputFloat("Camera rotation speed", &camera_rotation_speed))
		App->camera->camera_rotation_speed = camera_rotation_speed;
}

void ModuleUI::DrawTimeControlWindow()
{
	ImGui::Begin("Time control", &open_tabs[TIME_CONTROL]);

	int w, h;
	ui_textures[PLAY]->getSize(w, h);
	if (ImGui::ImageButton((void*)ui_textures[PLAY]->getGLid(), ImVec2(w, h), nullptr, ImVec2(0,0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, App->time->getGameState() == PLAYING ? 1.0f : 0.0f)))
		App->time->Play();

	ImGui::SameLine();
	ui_textures[PAUSE]->getSize(w, h);
	if(ImGui::ImageButton((void*)ui_textures[PAUSE]->getGLid(), ImVec2(w, h), nullptr, ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, App->time->getGameState() == PAUSED ? 1.0f : 0.0f)))
		App->time->Pause();

	ImGui::SameLine();
	ui_textures[STOP]->getSize(w, h);
	if (ImGui::ImageButton((void*)ui_textures[STOP]->getGLid(), ImVec2(w, h), nullptr, ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, App->time->getGameState() == STOPPED ? 1.0f : 0.0f)))
		App->time->Stop();
	

	static int advance_frames = 1;
	static float time_scale = 1;

	ImGui::SameLine();
	ui_textures[ADVANCE]->getSize(w, h);
	if (ImGui::ImageButton((void*)ui_textures[ADVANCE]->getGLid(), ImVec2(w, h), nullptr, ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.7f, 0.7f, 0.0f)))
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

void ModuleUI::DrawGizmoMenuTab() {

	ImGui::Begin("Toolbar##Gizmo toolbar", nullptr);

	if (App->camera->background_camera->getFrustum()->type == math::FrustumType::OrthographicFrustum)
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

void ModuleUI::DrawQuadtreeConfigWindow() {

	ImGui::Begin("Quadtree", &open_tabs[QUADTREE_CONFIG]);
	ImGui::Checkbox("Draw", &App->scene->draw_quadtree);
	ImGui::SameLine();
	if (ImGui::Button("Reload")) {
		App->scene->quadtree_reload = true;
	}
	static float3 size = float3(0,0,0);
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
	ImGui::Text("Quadtree ignored objects: %i",App->scene->quadtree_ignored_obj);
	ImGui::Text("Frustum checks against quadtree: %i", App->scene->quadtree_checks);
	// TODO: Be able to change bucket size, max depth and size.
	ImGui::End();
}


void ModuleUI::DrawGuizmo()
{

	App->gui->DrawGizmoMenuTab();

	if (draw_guizmo && App->camera->background_camera->getFrustum()->type != math::FrustumType::OrthographicFrustum)
	{
		ImGuizmo::BeginFrame();
		float4x4 projection4x4;
		float4x4 view4x4;

		glGetFloatv(GL_MODELVIEW_MATRIX, (float*)view4x4.v);
		glGetFloatv(GL_PROJECTION_MATRIX, (float*)projection4x4.v);

		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

		ComponentTransform* transform = (ComponentTransform*)(*App->scene->selected_obj.begin())->getComponent(TRANSFORM);
		Transform* trans = transform->global;

		Transform aux_transform;
		switch (gizmo_operation)
		{
		case ImGuizmo::OPERATION::TRANSLATE:
			aux_transform.setRotation(trans->getRotation());
			aux_transform.setPosition(trans->getPosition()); break;
		case ImGuizmo::OPERATION::ROTATE:
			aux_transform.setPosition(trans->getPosition());
			aux_transform.setRotation(trans->getRotation()); break;
		case ImGuizmo::OPERATION::SCALE:
			aux_transform.setPosition(trans->getPosition());
			aux_transform.setRotation(trans->getRotation());
			aux_transform.setScale(trans->getScale()); break;
		default:
			break;
		}
		
		aux_transform.CalculateMatrix();
		float4x4 mat = float4x4(aux_transform.getMatrix());
		mat.Transpose();
		ImGuizmo::Manipulate((float*)view4x4.v, (float*)projection4x4.v, gizmo_operation, gizmo_mode, (float*)mat.v);
		if (ImGuizmo::IsUsing())
		{
			float3 new_pos = float3::zero;
			float3 new_rot = float3::zero;
			float3 new_scale = float3::zero;
			mat.Transpose();
			switch (gizmo_operation)
			{
			case ImGuizmo::OPERATION::TRANSLATE:
				new_pos.x = transform->constraints[0][0] ? trans->getPosition().x : (mat.TranslatePart().x);
				new_pos.y = transform->constraints[0][1] ? trans->getPosition().y : (mat.TranslatePart().y);
				new_pos.z = transform->constraints[0][2] ? trans->getPosition().z : (mat.TranslatePart().z);
				trans->setPosition(new_pos);
				break;
			case ImGuizmo::OPERATION::ROTATE:
				new_rot.x = transform->constraints[1][0] ? trans->getRotationEuler().x : mat.RotatePart().ToEulerXYZ().x;
				new_rot.y = transform->constraints[1][1] ? trans->getRotationEuler().y : mat.RotatePart().ToEulerXYZ().y;
				new_rot.z = transform->constraints[1][2] ? trans->getRotationEuler().z : mat.RotatePart().ToEulerXYZ().z;
				trans->setRotation(Quat::FromEulerXYZ(new_rot.x, new_rot.y, new_rot.z));
				break;
			case ImGuizmo::OPERATION::SCALE:
				new_scale.x = transform->constraints[2][0] ? trans->getScale().x : mat.GetScale().x;
				new_scale.y = transform->constraints[2][1] ? trans->getScale().y : mat.GetScale().y;
				new_scale.z = transform->constraints[2][2] ? trans->getScale().z : mat.GetScale().z;
				trans->setScale(new_scale);
				break;
			default:
				break;
			}
			trans->CalculateMatrix();
			transform->GlobalToLocal();
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
	json_object_set_boolean(config, "hierarchy", open_tabs[HIERARCHY]);
	json_object_set_boolean(config, "obj_inspector", open_tabs[OBJ_INSPECTOR]);
	json_object_set_boolean(config, "primitive", open_tabs[PRIMITIVE]);
	json_object_set_boolean(config, "about", open_tabs[ABOUT]);
	json_object_set_boolean(config, "configuration", open_tabs[CONFIGURATION]);
	json_object_set_boolean(config, "log", open_tabs[LOG]);
	json_object_set_boolean(config, "time_control", open_tabs[TIME_CONTROL]);
	json_object_set_boolean(config, "quadtree_config", open_tabs[QUADTREE_CONFIG]);
	json_object_set_boolean(config, "camera_menu", open_tabs[CAMERA_MENU]);
	json_object_set_boolean(config, "asset_window", open_tabs[ASSET_WINDOW]);
	json_object_set_boolean(config, "resources_window", open_tabs[RESOURCES_TAB]);
	//json_object_set_boolean(config, "audio", open_tabs[AUDIO]);
}

void ModuleUI::LoadConfig(const JSON_Object* config) 
{
	open_tabs[CONFIGURATION]	= json_object_get_boolean(config, "configuration");
	open_tabs[HIERARCHY]		= json_object_get_boolean(config, "hierarchy");
	open_tabs[OBJ_INSPECTOR]	= json_object_get_boolean(config, "obj_inspector");
	open_tabs[PRIMITIVE]		= json_object_get_boolean(config, "primitive");
	open_tabs[ABOUT]			= json_object_get_boolean(config, "about");
	open_tabs[LOG]				= json_object_get_boolean(config, "log");
	open_tabs[TIME_CONTROL]		= json_object_get_boolean(config, "time_control");
	open_tabs[QUADTREE_CONFIG]	= json_object_get_boolean(config, "quadtree_config");
	open_tabs[CAMERA_MENU]		= json_object_get_boolean(config, "camera_menu");
	open_tabs[ASSET_WINDOW]		= json_object_get_boolean(config, "asset_window");
	open_tabs[RESOURCES_TAB]	= json_object_get_boolean(config, "resources_window");


	open_tabs[VIEWPORT_MENU]	= false;	// must always start closed
	open_tabs[SKYBOX_MENU]		= false;
	open_tabs[SCRIPT_EDITOR] = false;
	//open_tabs[AUDIO]			= json_object_get_boolean(config, "audio");
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
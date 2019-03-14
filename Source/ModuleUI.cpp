 #include "ModuleUI.h"
#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleCamera3D.h"
#include "ModuleRenderer3D.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleImporter.h"
#include "ModuleExporter.h"
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
#include "ComponentProgressBarUI.h"
#include "ComponentButtonUI.h"
#include "ComponentAnimation.h"
#include "ComponentBillboard.h"
#include "ComponentAnimator.h"
#include "ComponentPhysics.h"
#include "Transform.h"
#include "Camera.h"
#include "Quadtree.h"
#include "ResourceTexture.h"
#include "Resource3dObject.h"
#include "ResourceAnimation.h"
#include "ResourceMesh.h"
#include "ResourceAnimationGraph.h"
#include "Skybox.h"
#include "FileSystem.h"
#include "Include_Wwise.h"
#include "ComponentAnimationEvent.h"

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

// Include Panels
#include "Panel.h"
#include "PanelAnimation.h"
#include "PanelAnimationEvent.h"
#include "PanelHierarchyTab.h"
#include "PanelObjectInspector.h"
#include "PanelAssetsWin.h"
#include "PanelPrimitives.h"
#include "PanelAnimationGraph.h"
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

	p_anim = new PanelAnimation("AnimEditor");
	p_anim_evt = new PanelAnimationEvent("AnimEvtEditor");
	p_hierarchy = new PanelHierarchyTab("Hierarchy", true);
	p_inspector = new PanelObjectInspector("Object Inspector", true);
	p_assetswindow = new PanelAssetsWin("Assets", true);
	p_primitives = new PanelPrimitives("Primitives", true);
	p_animation_graph = new PanelAnimationGraph("Animation Graph", false);
	p_configuration = new PanelConfiguration("Configuration", true);
	p_time_control = new PanelTimeControl("Time Control", true);
	p_shader_editor = new PanelShader("Shader Editor", false);
	p_about = new PanelAbout("About");
	p_camera_menu = new PanelCameraMenu("Camera Menu");
	p_viewports = new PanelViewports("Viewports");
	p_quadtree_config = new PanelQuadtreeConfig("Quadtree Configuration");
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


	// HARDCODE
	//App->scene->AskSceneLoadFile("Assets/Scenes/animation.scene");

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

	//ImGui::ShowDemoWindow();
	return UPDATE_CONTINUE;
}

update_status ModuleUI::Update(float dt) {

	if (!App->is_game)
	{
		
		InvisibleDockingBegin();
		static bool file_save = false;
		disable_keyboard_control = false;

		if (p_configuration->isActive())
			p_configuration->Draw();

		if (p_hierarchy->isActive())
			p_hierarchy->Draw();

		if (p_inspector->isActive())
			p_inspector->Draw();

		if (p_primitives->isActive())
			p_primitives->Draw();

		if (p_about->isActive())
			p_about->Draw();

		if (open_log_menu)
			app_log->Draw("App log", &open_log_menu);

		if (p_time_control->isActive())
			p_time_control->Draw();

		if (p_quadtree_config->isActive())
			p_quadtree_config->Draw();

		if (p_camera_menu->isActive())
			p_camera_menu->Draw();

		if (p_viewports->isActive())
			p_viewports->Draw();

		if (p_assetswindow->isActive())
			p_assetswindow->Draw();

		if (open_tabs[SKYBOX_MENU])
			DrawSkyboxWindow();

		if (p_animation_graph->isActive())
			p_animation_graph->Draw();

		if (open_tabs[SCRIPT_EDITOR])
			DrawScriptEditor();

		if (open_tabs[BUILD_MENU])
			DrawBuildMenu();

		if (p_shader_editor->isActive())
			p_shader_editor->Draw();

		

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
				ImGui::MenuItem("Skybox", NULL, &open_tabs[SKYBOX_MENU]);
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
					App->requestBrowser("https://github.com/Skyway666/Kuroko-Engine/issues");
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
				ResourceMesh* R_mesh = (ResourceMesh*)App->resources->getResource(c_mesh->getMeshResource());
				ImGui::Text("Resource: %s", (R_mesh != nullptr) ? R_mesh->asset.c_str() : "None");

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

				if (ImGui::TreeNode("Connected Bones"))
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

			static bool game_camera;
			game_camera = (App->camera->game_camera == camera->getCamera());

			if (ImGui::Checkbox("Game Camera", &game_camera))
				App->camera->game_camera = game_camera ? camera->getCamera() : App->camera->editor_camera;

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
				if (ImGui::MenuItem("NONE"))
				{
					((ComponentAudioSource*)&component)->SetSoundID(0);
					((ComponentAudioSource*)&component)->SetSoundName("Sound");
					select_audio = false;
				}
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
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
			if (ImGui::DragFloat2("##p", (float*)&position, 0.01f)) { rectTrans->setPos(position); }
			//Width
			ImGui::Text("Dimensions:");
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

	case UI_PROGRESSBAR:
		if (ImGui::CollapsingHeader("UI Progress Bar"))
		{
			ComponentProgressBarUI* pbar = (ComponentProgressBarUI*)&component;
			static float2 position = pbar->getPos();
			static float percent = pbar->getPercent();
			static float width = pbar->getInteriorWidth();
			static float depth = pbar->getInteriorDepth();

			ImGui::Text("Percent:");
			ImGui::DragFloat("##d", (float*)&percent, 1, 0, 100); {pbar->setPercent(percent); }
			ImGui::Text("Position:");
			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
			if (ImGui::DragFloat2("##ps", (float*)&position, 0.01f)) { pbar->setPos(position); }
			ImGui::Text("Interior bar width:");
			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
			if (ImGui::DragFloat("##wi", (float*)&width, 0.1f)) { pbar->setInteriorWidth(width); }
			ImGui::Text("Interior bar depth:");
			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
			if (ImGui::DragFloat("##dp", (float*)&depth, 0.1f)) { pbar->setInteriorDepth(depth); }

			int w = 0; int h = 0;
			if (pbar->getResourceTexture() != nullptr) {
				pbar->getResourceTexture()->texture->getSize(w, h);
			}

			ImGui::Text("Bar texture data: \n x: %d\n y: %d", w, h);

			if (ImGui::Button("Load(from asset folder)##Dif: Loadtex"))
			{
				std::string texture_path = openFileWID();
				uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
				if (new_resource != 0) {
					App->resources->assignResource(new_resource);
					if (pbar->getResourceTexture() != nullptr)
						App->resources->deasignResource(pbar->getResourceTexture()->uuid);
					pbar->setResourceTexture((ResourceTexture*)App->resources->getResource(new_resource));
				}
			}

			ImGui::Image(pbar->getResourceTexture() != nullptr ? (void*)pbar->getResourceTexture()->texture->getGLid() : (void*)ui_textures[NO_TEXTURE]->getGLid(), ImVec2(128, 128));


			int w2 = 0; int h2 = 0;
			if (pbar->getResourceTextureInterior() != nullptr) {
				pbar->getResourceTextureInterior()->texture->getSize(w2, h2);
			}


			ImGui::Text("Interior Bar texture data: \n x: %d\n y: %d", w2, h2);

			if (ImGui::Button("Load(from asset folder)##Dif: Loadtex2"))
			{
				std::string texture_path = openFileWID();
				uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
				if (new_resource != 0) {
					App->resources->assignResource(new_resource);
					if (pbar->getResourceTextureInterior() != nullptr)
						App->resources->deasignResource(pbar->getResourceTextureInterior()->uuid);
					pbar->setResourceTextureInterior((ResourceTexture*)App->resources->getResource(new_resource));
				}
			}

			ImGui::Image(pbar->getResourceTextureInterior() != nullptr ? (void*)pbar->getResourceTextureInterior()->texture->getGLid() : (void*)ui_textures[NO_TEXTURE]->getGLid(), ImVec2(128, 128));
			ImGui::SameLine();
		}
		break;
	case ANIMATION:
		if (ImGui::CollapsingHeader("Animation"))
		{

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

			if (ImGui::Button("AnimEditor"))
				p_anim->toggleActive();
			if (p_anim->isActive())
				p_anim->Draw();

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
			ImGui::Checkbox("Script Controlled", &c_emitter->script_controlled);


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

				static bool local_particles = false;
				local_particles = c_emitter->transform_mode == LOCAL;

				if (ImGui::Checkbox("Local", &local_particles))
					c_emitter->transform_mode = local_particles ? LOCAL : GLOBAL;

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
	case ANIMATION_EVENT:
	{
		if (ImGui::CollapsingHeader("Animation Events"))
		{

			ComponentAnimationEvent* anim_evt = nullptr;
			anim_evt = (ComponentAnimationEvent*)&component;
			//ResourceAnimation* R_anim = (ResourceAnimation*)App->resources->getResource(anim->getAnimationResource());
			//ImGui::Text("Resource: %s", (R_anim != nullptr) ? R_anim->asset.c_str() : "None");

			static bool set_animation_menu = false;

			
			if (ImGui::Button("Create Animation"))
				p_anim_evt->new_anim_set_win = true;
			if (p_anim_evt->new_anim_set_win)
			{
				ImGui::Begin("Name", &p_anim_evt->new_anim_set_win);

				ImGui::InputText("#SetName", p_anim_evt->prov, 50);
				if (ImGui::Button("Create"))
				{
					AnimSet push;
					push.name = p_anim_evt->prov;
					p_anim_evt->new_anim_set_win = false;


					anim_evt->AnimEvts.push_back(push);
					if(anim_evt->curr != nullptr)
						anim_evt->curr->selected = false;
					anim_evt->curr = &anim_evt->AnimEvts.back();
					p_anim_evt->curr = --anim_evt->AnimEvts.end();
					//p_anim_evt->prov = "\0";
				}

				ImGui::End();
			}
			
			if (anim_evt->curr == nullptr && anim_evt->AnimEvts.size() > 0)
			{
				if (ImGui::BeginCombo("Animation Sets", anim_evt->curr->name.c_str()))
				{

					for (auto it = anim_evt->AnimEvts.begin(); it != anim_evt->AnimEvts.end(); ++it)
						if (ImGui::Selectable(it->name.c_str(), &it->selected))
						{
							anim_evt->curr->selected = false;
							anim_evt->curr = &it._Ptr->_Myval;
							anim_evt->curr->selected = true;

							for (auto it = anim_evt->AnimEvts.begin(); it != anim_evt->AnimEvts.end(); ++it)
								if (&it._Ptr->_Myval == anim_evt->curr)
								{
									p_anim_evt->curr = it;
									break;
								}
						}
					ImGui::EndCombo();
				}

				ImGui::SameLine();
				if (ImGui::Button("Delete"))
				{
					anim_evt->AnimEvts.erase(p_anim_evt->curr);
					anim_evt->curr = nullptr;
					p_anim_evt->curr = anim_evt->AnimEvts.end();
				}
			}

			if (anim_evt->curr != nullptr)
			{
				if (ImGui::BeginCombo("Animation Sets##12", anim_evt->curr->name.c_str()))
				{

					for (auto it = anim_evt->AnimEvts.begin(); it != anim_evt->AnimEvts.end(); ++it)
						if (ImGui::Selectable(it->name.c_str(), &it->selected))
						{
							anim_evt->curr->selected = false;
							anim_evt->curr = &it._Ptr->_Myval;
							anim_evt->curr->selected = true;

							for (auto it = anim_evt->AnimEvts.begin(); it != anim_evt->AnimEvts.end(); ++it)
								if (&it._Ptr->_Myval == anim_evt->curr)
								{
									p_anim_evt->curr = it;
									break;
								}
						}
					ImGui::EndCombo();
				}
				
				if (ImGui::Button("Link Animation")) p_anim_evt->copy_specs_win = true;

				p_anim_evt->CopySpecs();
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35);
					auto get = component.getParent()->getComponent(Component_type::ANIMATION);
					if (get != nullptr)
						ImGui::TextUnformatted(("Link the component animation to the\nskeletal animation, if available\n Currently linked to: " + get->TypeToString()).c_str());
					else
						ImGui::TextUnformatted(("No Animation to link to"));
					ImGui::PopTextWrapPos();
					ImGui::EndTooltip();
				}

				static bool animation_active;
				animation_active = anim_evt->isActive();

				if (ImGui::Checkbox("Active##active animation evt", &animation_active))
					anim_evt->setActive(animation_active);

				ImGui::Checkbox("Loop", &anim_evt->curr->loop);


				ImGui::InputInt("Duration", &anim_evt->curr->own_ticks, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35);
					ImGui::TextUnformatted("In Frames");
					ImGui::PopTextWrapPos();
					ImGui::EndTooltip();
				}
				ImGui::InputInt("FrameRate", &anim_evt->curr->ticksXsecond, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);

				ImGui::InputFloat("Speed", &anim_evt->curr->speed, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35);
					ImGui::TextUnformatted("Animation Speed Multiplier");
					ImGui::PopTextWrapPos();
					ImGui::EndTooltip();
				}
				//if (R_anim != nullptr)
				{
					if (App->time->getGameState() != GameState::PLAYING)
					{
						ImGui::Text("Play");
						ImGui::SameLine();
						ImGui::Text("Pause");
					}
					else if (anim_evt->isPaused())
					{
						if (ImGui::Button("Play"))
							anim_evt->Play();
						ImGui::SameLine();
						ImGui::Text("Pause");
					}
					else
					{
						ImGui::Text("Play");
						ImGui::SameLine();
						if (ImGui::Button("Pause"))
							anim_evt->Pause();
					}

					ImGui::Text("Animation info:");
					if(anim_evt->curr->ticksXsecond != 0)
						ImGui::Text("Duration: %.1f ms", anim_evt->curr->own_ticks / anim_evt->curr->ticksXsecond * 1000.f);
					//ImGui::Text(" Animation Bones: %d", R_anim->numBones);
				}

				if (ImGui::Button("AnimEditor"))
					p_anim_evt->toggleActive();
				if (p_anim_evt->isActive())
					p_anim_evt->Draw();
			}

			if (ImGui::Button("Remove Component##Remove animation"))
				ret = false;			
		}
		break;
	}
	case ANIMATOR:
		if (ImGui::CollapsingHeader("Animator"))
		{
			ComponentAnimator* animator = (ComponentAnimator*)&component;
			ResourceAnimationGraph* R_graph = (ResourceAnimationGraph*)App->resources->getResource(animator->getAnimationGraphResource());
			ImGui::Text("Resource: %s", (R_graph != nullptr) ? R_graph->asset.c_str() : "None");

			static bool set_animation_menu = false;
			if (ImGui::Button((R_graph != nullptr) ? "Change Animation Graph" : "Add Animation Graph")) {
				set_animation_menu = true;
			}

			if (set_animation_menu) {

				std::list<resource_deff> graph_res;
				App->resources->getAnimationGraphResourceList(graph_res);

				ImGui::Begin("Animation selector", &set_animation_menu);
				for (auto it = graph_res.begin(); it != graph_res.end(); it++) {
					resource_deff anim_deff = (*it);
					if (ImGui::MenuItem(anim_deff.asset.c_str())) {
						animator->setAnimationGraphResource(anim_deff.uuid);
						set_animation_menu = false;
						break;
					}
				}

				ImGui::End();
			}

			static bool animator_active;
			animator_active = animator->isActive();

			if (ImGui::Checkbox("Active##active animator", &animator_active))
				animator->setActive(animator_active);
		}
		break;
	case PHYSICS:
	{	if (ImGui::CollapsingHeader("Collider"))
		{
		ImGui::TextWrapped("Drag the parameters to change them, or ctrl+click on one of them to set it's value");
		ComponentPhysics* c_phys = (ComponentPhysics*)&component;

		bool toggle_static = c_phys->is_environment;

		//position
		ImGui::Text("Offset:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##p x", &c_phys->offset_pos.x, 0.01f, 0.0f, 0.0f, "%.02f");

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##p y", &c_phys->offset_pos.y, 0.01f, 0.0f, 0.0f, "%.02f");

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##p z", &c_phys->offset_pos.z, 0.01f, 0.0f, 0.0f, "%.02f");

		//rotation
		ImGui::Text("Rotation:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##r x", &c_phys->offset_rot.x, 0.2f, -180.0f, 180.0f, "%.02f");

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##r y", &c_phys->offset_rot.y, 0.2f, -180.0f, 180.0f, "%.02f");

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##r z", &c_phys->offset_rot.z, 0.2f, -180.0f, 180.0f, "%.02f");

		//scale
		ImGui::Text("   Scale:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##s x", &c_phys->offset_scale.x, 0.01f, -1000.0f, 1000.0f, "%.02f");

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##s y", &c_phys->offset_scale.y, 0.01f, -1000.0f, 1000.0f, "%.02f");

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##s z", &c_phys->offset_scale.z, 0.01f, -1000.0f, 1000.0f, "%.02f");

		ImGui::Checkbox("is environment", &toggle_static);
		if (toggle_static != c_phys->is_environment)
		{
			c_phys->SetStatic(toggle_static);
		}

		}
	}
	break;
	case TRIGGER:
	{	if (ImGui::CollapsingHeader("Trigger"))
	{
		ImGui::TextWrapped("Drag the parameters to change them, or ctrl+click on one of them to set it's value");
		ComponentPhysics* c_phys = (ComponentPhysics*)&component;

		//position
		ImGui::Text("Offset:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##p x", &c_phys->offset_pos.x, 0.01f, 0.0f, 0.0f, "%.02f");

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##p y", &c_phys->offset_pos.y, 0.01f, 0.0f, 0.0f, "%.02f");

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##p z", &c_phys->offset_pos.z, 0.01f, 0.0f, 0.0f, "%.02f");

		//rotation
		ImGui::Text("Rotation:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##r x", &c_phys->offset_rot.x, 0.2f, -180.0f, 180.0f, "%.02f");

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##r y", &c_phys->offset_rot.y, 0.2f, -180.0f, 180.0f, "%.02f");

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##r z", &c_phys->offset_rot.z, 0.2f, -180.0f, 180.0f, "%.02f");

		//scale
		ImGui::Text("   Scale:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##s x", &c_phys->offset_scale.x, 0.01f, -1000.0f, 1000.0f, "%.02f");

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##s y", &c_phys->offset_scale.y, 0.01f, -1000.0f, 1000.0f, "%.02f");

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##s z", &c_phys->offset_scale.z, 0.01f, -1000.0f, 1000.0f, "%.02f");

	}
	}
	break;
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

		if (App->camera->game_camera == &camera)
			window_name += "(Game camera)";

		ImGui::Begin(window_name.c_str(), &camera.draw_in_UI, ImGuiWindowFlags_NoScrollbar);
		
		ImVec2 window_pos = ImGui::GetItemRectMin();


		ImVec2 window_size = ImGui::GetWindowSize();
		window_size.x -= CAMERA_VIEW_MARGIN_X;
		window_size.y -= CAMERA_VIEW_MARGIN_Y;

		frame_buffer->changeTexSize(window_size.x, window_size.y);

		if (ImGui::ImageButton((void*)(camera.draw_depth ? frame_buffer->depth_tex->gl_id : frame_buffer->tex->gl_id), window_size, nullptr, ImVec2(0, 1), ImVec2(1, 0), -1))
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
			if (!App->scene->selected_obj.empty() && !App->scene->selected_obj.front()->isStatic() && !App->scene->selected_obj.front()->is_UI) // Not draw guizmo if it is static
			{
				ImGuizmo::SetDrawlist();
				App->gui->DrawGuizmo(window_pos, window_size);
			}

		ImGui::End();
	}
	else 
		camera.initFrameBuffer();

	if (&camera == App->camera->editor_camera)
		if (!App->scene->selected_obj.empty() && !App->scene->selected_obj.front()->isStatic() && !App->scene->selected_obj.front()->is_UI)
			App->gui->DrawGizmoMenuTab();

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
	if (building)
	{
		building = false;
		errorBuilding = !App->exporter->CreateBuild(buildPath.c_str(), buildName);
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
		float4x4 projection4x4;
		float4x4 view4x4;

		glGetFloatv(GL_MODELVIEW_MATRIX, (float*)view4x4.v);
		glGetFloatv(GL_PROJECTION_MATRIX, (float*)projection4x4.v);

		ImGuizmo::SetRect(window_pos.x, window_pos.y, window_size.x, window_size.y);

		ComponentTransform* transform = (ComponentTransform*)(*App->scene->selected_obj.begin())->getComponent(TRANSFORM);
		Transform* trans = transform->global;

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
		mat.Transpose();
		ImGuizmo::Manipulate((float*)view4x4.v, (float*)projection4x4.v, gizmo_operation, gizmo_mode, (float*)mat.v);
		if (ImGuizmo::IsUsing())
		{
			float3 new_pos = float3::zero;
			float3 new_rot = float3::zero;
			float3 new_scale = float3::zero;
			mat.Transpose();

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
						trans->setPosition(new_pos);
						break;
					case ImGuizmo::OPERATION::ROTATE:
						if (App->scene->selected_obj.size() > 1)
						{
							new_rot.x = transform->constraints[1][0] ? trans->getRotationEuler().x : mat.RotatePart().ToEulerXYZ().x;
							new_rot.y = transform->constraints[1][1] ? trans->getRotationEuler().y : mat.RotatePart().ToEulerXYZ().y;
							new_rot.z = transform->constraints[1][2] ? trans->getRotationEuler().z : mat.RotatePart().ToEulerXYZ().z;
							trans->setRotation(Quat::FromEulerXYZ(new_rot.x, new_rot.y, new_rot.z) * trans->getRotation());
						}
						else
						{
							new_rot.x = transform->constraints[1][0] ? trans->getRotationEuler().x : mat.RotatePart().ToEulerXYZ().x;
							new_rot.y = transform->constraints[1][1] ? trans->getRotationEuler().y : mat.RotatePart().ToEulerXYZ().y;
							new_rot.z = transform->constraints[1][2] ? trans->getRotationEuler().z : mat.RotatePart().ToEulerXYZ().z;
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
				//-----
			}
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

	open_tabs[SKYBOX_MENU]		= false;
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
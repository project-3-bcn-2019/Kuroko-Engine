#ifndef __MODULE_IMGUI
#define __MODULE_IMGUI

#include "Module.h"
#include "Globals.h"
#include "ImGui/imgui.h"
#include "ImGui\ImGuizmo.h"
#include "ComponentCamera.h"
#include "FileSystem.h"
#include <array>
#include <list>
#include "ImGui/TextEditor.h"

struct ImGuiIO;
class GameObject;
class Component;
class Texture;
class Material;
class Camera;
struct Color;
class TextEditor;

// Panel Classes
class Panel;
class PanelAnimation;
class PanelAnimationEvent;
class PanelHierarchyTab;
class PanelObjectInspector;
class PanelAssetsWin;
class PanelPrimitives;
class PanelAnimationGraph;
class PanelConfiguration;
class PanelTimeControl;
class PanelShader;
class PanelAbout;
class PanelCameraMenu;
class PanelViewports;
class PanelQuadtreeConfig;
class PanelSkyboxWin;

enum GUI_Tabs { SCRIPT_EDITOR, BUILD_MENU, LAST_UI_TAB };  
				// LAST is an utility value to store the max num of tabs.

enum UI_textures { NO_TEXTURE, PLAY, PAUSE, STOP, ADVANCE, GUIZMO_TRANSLATE, GUIZMO_ROTATE, GUIZMO_SCALE, GUIZMO_LOCAL, GUIZMO_GLOBAL, 
					GUIZMO_SELECT, FOLDER_ICON, OBJECT_ICON, SCENE_ICON, SCRIPT_ICON, PREFAB_ICON, RETURN_ICON, CAUTION_ICON,
					WARNING_ICON, GRAPH_ICON, AUDIO_ICON,SHADER_ICON, LAST_UI_TEX};

enum UI_Fonts {REGULAR, REGULAR_BOLD, REGULAR_ITALIC, REGULAR_BOLDITALIC, TITLES, IMGUI_DEFAULT, LAST_UI_FONT};

class ModuleUI :
	public Module {
public:
	ModuleUI(Application* app, bool start_enabled = true);
	~ModuleUI();

	bool Init(const JSON_Object* config);
	bool Start();
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);
	bool CleanUp();
	void InitializeScriptEditor();

	//bool DrawComponent(Component& component, int id);
	void DrawCameraViewWindow(Camera& camera);
	void DrawGizmoMenuTab();
	/*void DrawSkyboxWindow();*/
	void DrawColorPickerWindow(const char* label, Color* color, bool* closing_bool, Color* ref_color = nullptr);
	void DrawScriptEditor();
	void DrawBuildMenu();

	uint getMainScene() const;
	std::list<resource_deff> GetBuildScenes() { return build_scenes; }

	void DrawGuizmo(ImVec2 window_pos, ImVec2 window_size);
	void DrawTagSelection(GameObject* object);

	void InvisibleDockingBegin();
	void InvisibleDockingEnd();

	void SaveConfig(JSON_Object* config) const;
	void LoadConfig(const JSON_Object* config);

	bool isMouseOnUI() const;
	bool keepKeyboard() const;

	bool disable_keyboard_control = false;

	// Panels
	std::list<Panel*> panels;
	PanelAnimation* p_anim = nullptr;
	PanelAnimationEvent* p_anim_evt = nullptr;
	PanelHierarchyTab* p_hierarchy = nullptr;
	PanelObjectInspector* p_inspector = nullptr;
	PanelAssetsWin* p_assetswindow = nullptr;
	PanelPrimitives* p_primitives = nullptr;
	PanelAnimationGraph* p_animation_graph = nullptr;
	PanelConfiguration* p_configuration = nullptr;
	PanelTimeControl* p_time_control = nullptr;
	PanelShader* p_shader_editor = nullptr;
	PanelAbout* p_about = nullptr;
	PanelCameraMenu* p_camera_menu = nullptr;
	PanelViewports* p_viewports = nullptr;
	PanelQuadtreeConfig* p_quadtree_config = nullptr;
	PanelSkyboxWin* p_skybox = nullptr;

public:
	TextEditor script_editor; //USED IN SCRIPT EDITOR & ASSETS WINDOW
	std::string open_script_path; //USED IN SCRIPT EDITOR & ASSETS WINDOW

	bool open_tabs[LAST_UI_TAB];  // _serializable_var

	std::array<Texture*, LAST_UI_TEX> ui_textures;
	std::array<ImFont*, LAST_UI_FONT> ui_fonts;
	ImVec2 game_window_pos = { 0.0f, 0.0f };

private:
	
	bool docking_background = true;
	bool draw_guizmo = true;
	bool using_guizmo = false;

	ImGuiIO* io;
	ImGuizmo::OPERATION	gizmo_operation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE gizmo_mode = ImGuizmo::WORLD;


	std::string asset_window_path = ASSETS_FOLDER; //NOT IN USE I GUESS---------------(yes it is, in DrawAssetWindow)
	std::string selected_asset; //NOT IN USE I GUESS--------------- (yes it is, in DrawAssetWindow)

	std::list<resource_deff> build_scenes;
	std::vector<bool> main_scene;

	bool open_log_menu = false; //To avoid the GUI_TABS enum
};

#endif


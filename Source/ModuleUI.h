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

enum GUI_Tabs { HIERARCHY, OBJ_INSPECTOR, PRIMITIVE, ABOUT, LOG, TIME_CONTROL, CONFIGURATION,
				QUADTREE_CONFIG, CAMERA_MENU, VIEWPORT_MENU /*AUDIO,*/, ASSET_WINDOW, RESOURCES_TAB, SKYBOX_MENU, SCRIPT_EDITOR, BUILD_MENU, LAST_UI_TAB };  
				// LAST is an utility value to store the max num of tabs.

enum UI_textures { NO_TEXTURE, PLAY, PAUSE, STOP, ADVANCE, GUIZMO_TRANSLATE, GUIZMO_ROTATE, GUIZMO_SCALE, GUIZMO_LOCAL, GUIZMO_GLOBAL, 
					GUIZMO_SELECT, FOLDER_ICON, OBJECT_ICON, SCENE_ICON, SCRIPT_ICON, PREFAB_ICON, RETURN_ICON, CAUTION_ICON,
					WARNING_ICON, GRAPH_ICON, AUDIO_ICON, LAST_UI_TEX};

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

	void DrawHierarchyTab();									//PANEL DONE
	bool DrawHierarchyNode(GameObject& game_object, int& id);	//PANEL DONE
	void DrawObjectInspectorTab();								//PANEL DONE
	bool DrawComponent(Component& component, int id);
	//void DrawAudioTab();
	void DrawPrimitivesTab();									//PANEL DONE
	void DrawGraphicsLeaf() const;
	void DrawAboutLeaf();
	void DrawWindowConfigLeaf() const;
	void DrawHardwareLeaf() const;
	void DrawApplicationLeaf() const;
	void DrawEditorPreferencesLeaf() const;
	void DrawTimeControlWindow();
	void DrawCameraViewWindow(Camera& camera);
	void DrawGizmoMenuTab();
	void DrawQuadtreeConfigWindow();
	void DrawCameraMenuWindow();
	void DrawViewportsWindow();
	void DrawAssetsWindow();									//PANEL DONE
	void DrawAssetInspector();									//PANEL DONE
	void DrawResourcesWindow(); // A list where you can see all the resources
	void DrawSkyboxWindow();
	void DrawColorPickerWindow(const char* label, Color* color, bool* closing_bool, Color* ref_color = nullptr);
	void DrawScriptEditor();
	void DrawBuildMenu();

	uint getMainScene() const;

	void DrawGuizmo();
	void DrawTagSelection(GameObject* object);

	void InvisibleDockingBegin();
	void InvisibleDockingEnd();

	void SaveConfig(JSON_Object* config) const;
	void LoadConfig(const JSON_Object* config);

	bool isMouseOnUI() const;

	bool disable_keyboard_control = false;

	// Panels
	PanelAnimation* p_anim = nullptr;
	PanelAnimationEvent* p_anim_evt = nullptr;
	PanelHierarchyTab* p_hierarchy = nullptr;
	PanelObjectInspector* p_inspector = nullptr;
	PanelAssetsWin* p_assetswindow = nullptr;
	PanelPrimitives* p_primitives = nullptr;
	PanelAnimationGraph* p_animation_graph = nullptr;

public:
	TextEditor script_editor; //USED IN SCRIPT EDITOR & ASSETS WINDOW
	std::string open_script_path; //USED IN SCRIPT EDITOR & ASSETS WINDOW

	bool open_tabs[LAST_UI_TAB];  // _serializable_var

	std::array<Texture*, LAST_UI_TEX> ui_textures;

private:
	
	bool docking_background = true;
	bool draw_guizmo = true;

	ImGuiIO* io;
	ImGuizmo::OPERATION	gizmo_operation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE gizmo_mode = ImGuizmo::WORLD;

	std::array<ImFont*, LAST_UI_FONT> ui_fonts;

	std::string asset_window_path = ASSETS_FOLDER; //NOT IN USE I GUESS---------------
	std::string selected_asset; //NOT IN USE I GUESS---------------

	std::list<resource_deff> build_scenes;
	std::vector<bool> main_scene;
};
#endif


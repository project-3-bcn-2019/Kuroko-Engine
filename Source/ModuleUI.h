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

enum GUI_Tabs { HIERARCHY, OBJ_INSPECTOR, PRIMITIVE, ABOUT, LOG, TIME_CONTROL, CONFIGURATION,
				QUADTREE_CONFIG, CAMERA_MENU, VIEWPORT_MENU /*AUDIO,*/, ASSET_WINDOW, RESOURCES_TAB, SKYBOX_MENU, SCRIPT_EDITOR, BUILD_MENU, SHADER_EDITOR, LAST_UI_TAB };  
				// LAST is an utility value to store the max num of tabs.

enum UI_textures { NO_TEXTURE, PLAY, PAUSE, STOP, ADVANCE, GUIZMO_TRANSLATE, GUIZMO_ROTATE, GUIZMO_SCALE, GUIZMO_LOCAL, GUIZMO_GLOBAL, 
					GUIZMO_SELECT, FOLDER_ICON, OBJECT_ICON, SCENE_ICON, SCRIPT_ICON, PREFAB_ICON, RETURN_ICON, CAUTION_ICON,
					WARNING_ICON, LAST_UI_TEX};

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
	void InitializeShaderEditor();

	void DrawHierarchyTab();
	bool DrawHierarchyNode(GameObject& game_object, int& id);
	void DrawObjectInspectorTab();
	bool DrawComponent(Component& component, int id);
	//void DrawAudioTab();
	void DrawPrimitivesTab();
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
	void DrawAssetsWindow();
	void DrawAssetInspector();
	void DrawResourcesWindow(); // A list where you can see all the resources
	void DrawSkyboxWindow();
	void DrawColorPickerWindow(const char* label, Color* color, bool* closing_bool, Color* ref_color = nullptr);
	void DrawScriptEditor();
	void DrawShaderEditor();
	void DrawBuildMenu();

	void DrawGuizmo();
	void DrawTagSelection(GameObject* object);


	void InvisibleDockingBegin();
	void InvisibleDockingEnd();

	void SaveConfig(JSON_Object* config) const;
	void LoadConfig(const JSON_Object* config);

	bool disable_keyboard_control = false;

	// Panels
	PanelAnimation* p_anim;

private:
	
	bool docking_background = true;
	bool draw_guizmo = true;

	ImGuiIO* io;
	ImGuizmo::OPERATION	gizmo_operation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE gizmo_mode = ImGuizmo::WORLD;

	TextEditor script_editor;
	TextEditor shader_editor;
	std::string open_script_path;
	std::string open_shader_path;

	bool open_tabs[LAST_UI_TAB];  // _serializable_var
	std::array<Texture*, LAST_UI_TEX> ui_textures;
	std::array<ImFont*, LAST_UI_FONT> ui_fonts;

	std::string asset_window_path = ASSETS_FOLDER;
	std::string selected_asset;

	std::list<std::string> build_scenes;
	std::vector<bool> main_scene;
};
#endif


#ifndef __MODULE_IMGUI
#define __MODULE_IMGUI
#include "Module.h"
#include "Globals.h"
#include "ImGui/imgui.h"
#include <array>

struct ImGuiIO;
class GameObject;
class Component;
class Texture;
class Material;

enum GUI_Tabs { DEMO, TEST, HIERARCHY, OBJ_INSPECTOR, PRIMITIVE, ABOUT, LOG, TIME_CONTROL, CONFIGURATION, /*AUDIO,*/ LAST};  // LAST is an utility value to store the max num of tabs.
enum UI_textures { PLAY, PAUSE, STOP, NO_TEXTURE, LAST_UI_TEX};

class ModuleImGUI :
	public Module {
public:
	ModuleImGUI(Application* app, bool start_enabled = true);
	~ModuleImGUI();

	bool Init(const JSON_Object& config);
	bool Start();
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);
	bool CleanUp();

	void DrawGraphicsTab() const;
	void DrawHierarchyTab();
	void DrawHierarchyNode(const GameObject& game_object, int& id) const;
	void DrawObjectInspectorTab();
	bool DrawComponent(Component& component);
	//void DrawAudioTab();
	void DrawPrimitivesTab() ;
	void DrawAboutWindow() ;
	void DrawWindowConfig() const;
	void DrawHardware() const;
	void DrawApplication() const;
	void DrawTimeControl();

	void InvisibleDockingBegin();
	void InvisibleDockingEnd();

	void SaveConfig(JSON_Object& config) const;
	void LoadConfig(const JSON_Object& config);

private:
	
	bool close_app = false;
	bool docking_background;
	bool open_tabs[LAST];  // _serializable_var
	ImGuiIO* io;
	std::array<Texture*, LAST_UI_TEX> ui_textures;
};
#endif


#ifndef __MODULE_SCENE
#define __MODULE_SCENE
#include "Module.h"
#include "MathGeoLib\MathGeoLib.h"
#include "MathGeoLib\Geometry\Frustum.h"
#include "MathGeoLib\Math\float3.h"
#include "ImGui\ImGuizmo.h"
#include "Transform.h"
#include <list>

class GameObject; 
class Material;
class Mesh;
class Texture;
class Skybox;
class Quadtree;


enum Component_type;


struct RayHit
{
	RayHit(GameObject*obj) : obj(obj) {};

	float distance = 0.0f;
	GameObject* obj = nullptr;
	float3 intersection_point = float3::zero;
};

class ModuleScene : public Module
{

	friend class ModuleResourcesManager;
public:
	ModuleScene(Application* app, bool start_enabled = true);
	~ModuleScene();

	bool Start();
	update_status PostUpdate(float dt);
	update_status Update(float dt);
	bool CleanUp();

	void DrawScene(float3 camera_pos);

	void addGameObject(GameObject* gobj)	{ game_objects.push_back(gobj); };
	GameObject* duplicateGameObject(GameObject* gobj);

	GameObject* getGameObject(uint uuid) const;
	std::list<GameObject*> getGameObjectsByTag(std::string tag);
	void getGameObjectsByComponent(Component_type type, std::list<GameObject*>& list_to_fill);
	Quadtree* getQuadtree() { return quadtree; }
	bool existingScene() { return working_on_existing_scene;}
	std::string getWorkigSceneName() { return current_working_scene; }

	void deleteGameObject(GameObject* gobj)	{ game_objs_to_delete.push_back(gobj); };
	void deleteGameObjectRecursive(GameObject* gobj);
	void ClearScene();

	void getRootObjs(std::list<GameObject*>& list_to_fill);

	// Script hot reloading
	void CleanScriptComponents();
	void LoadScriptComponents();

	GameObject* getCanvasGameObject();//creates gameobject with a canvas component if it's not created already (just 1 canvas needed)


	void AskPrefabLoadFile(const char* path, float3 pos, float3 eulerang);
	void SavePrefab(GameObject* root_obj, const char* name);
	void AskSceneSaveFile(char* scene_name); 
	void AskSceneLoadFile(char* path);
	void AskLocalSaveScene() { want_local_save = true; }
	void AskLocalLoadScene() { want_local_load = true; }


	GameObject* MousePicking(GameObject* ignore = nullptr);
	float3 MousePickingHit(GameObject* ignore = nullptr);

	GameObject* audiolistenerdefault = nullptr;
private:
	
	void ManageSceneSaveLoad();
	void SaveScene(std::string name);
	void LoadScene(const char* path);
	void LoadPrefab(const char* path);
	JSON_Value* serializeScene();
	JSON_Value* serializePrefab(GameObject* root_obj);

	void loadSerializedScene(JSON_Value* scene);
	void loadSerializedPrefab(JSON_Value* prefab);
private:

	std::list<GameObject*>	game_objects; 

	std::list<GameObject*>	game_objs_to_delete;

	Quadtree * quadtree		= nullptr;

	bool want_save_scene_file = false;
	bool want_load_scene_file = false;

	bool want_load_prefab_file = false;
	Transform prefab_load_spawn;
	bool want_local_save      = false;
	bool want_local_load	  = false;

	bool working_on_existing_scene	  = false;

	std::string current_working_scene = ""; // "" means that no scene is being edited

	std::string scene_to_save_name;
	std::string path_to_load_scene;
	std::string path_to_load_prefab;

	JSON_Value* local_scene_save = nullptr;		// To use when time starts and resumes

public:

	std::list<GameObject*> selected_obj;
	Skybox* skybox				= nullptr;

	uint last_mat_id			= 0;
	uint last_gobj_id			= 0;
	uint last_mesh_id			= 0;
	uint last_tex_id			= 0;

	bool global_wireframe		= false;
	bool global_normals			= false;
	// Dirty bools related to quadtee
	bool draw_quadtree			= true;
	bool quadtree_reload		= false;
	int quadtree_ignored_obj	= 0;
	int quadtree_checks			= 0;

	uint main_scene = 0;
};
#endif
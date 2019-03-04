#include "Application.h"
#include "ModuleScene.h"
#include "ModuleCamera3D.h"
#include "ImGui\imgui.h"
#include "Material.h"
#include "Mesh.h"
#include "ModuleInput.h"
#include "GameObject.h"		
#include "Camera.h"
#include "Skybox.h"
#include "Quadtree.h"
#include "ModuleRenderer3D.h"
#include "Applog.h"
#include "FileSystem.h"
#include "ModuleDebug.h"
#include "ModuleWindow.h"
#include "Component.h"
#include "ComponentAABB.h"
#include "ComponentTransform.h"
#include "Transform.h"
#include "ComponentMesh.h"
#include "ComponentScript.h"
#include "ComponentAudioListener.h"
#include "ComponentAudioSource.h"
#include "ModuleUI.h"
#include "ModuleResourcesManager.h"
#include "ModuleTimeManager.h"
#include "ModulePhysics3D.h"
#include "ModuleImporter.h" // TODO: remove this include and set skybox creation in another module (Importer?, delayed until user input?)
#include "Wwise_IDs.h"
#include "MathGeoLib\Geometry\LineSegment.h"
#include "glew-2.1.0\include\GL\glew.h"
#include "Random.h"

#include "ImGui\imgui.h"

#include <array>
#include <map>

ModuleScene::ModuleScene(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	name = "scene";
}

ModuleScene::~ModuleScene(){}

// Load assets
bool ModuleScene::Start()
{
	skybox = new Skybox();
	want_save_scene_file = false;
	want_load_scene_file = false;

	std::array<Texture*, 6> skybox_texs;
	if (!App->is_game || App->debug_game)
	{
		skybox_texs[LEFT] = (Texture*)App->importer->ImportTexturePointer("Assets/Textures/skybox_default_left.png");
		skybox_texs[RIGHT] = (Texture*)App->importer->ImportTexturePointer("Assets/Textures/skybox_default_right.png");
		skybox_texs[UP] = (Texture*)App->importer->ImportTexturePointer("Assets/Textures/skybox_default_up.png");
		skybox_texs[DOWN] = (Texture*)App->importer->ImportTexturePointer("Assets/Textures/skybox_default_down.png");
		skybox_texs[FRONT] = (Texture*)App->importer->ImportTexturePointer("Assets/Textures/skybox_default_front.png");
		skybox_texs[BACK] = (Texture*)App->importer->ImportTexturePointer("Assets/Textures/skybox_default_back.png");
	}
	else
	{
		skybox_texs[LEFT] = (Texture*)App->importer->ImportTexturePointer((TEXTURES_FOLDER + std::to_string(App->resources->getResourceUuid("skybox_default_left", R_TEXTURE)) + DDS_EXTENSION).c_str());
		skybox_texs[RIGHT] = (Texture*)App->importer->ImportTexturePointer((TEXTURES_FOLDER + std::to_string(App->resources->getResourceUuid("skybox_default_right", R_TEXTURE)) + DDS_EXTENSION).c_str());
		skybox_texs[UP] = (Texture*)App->importer->ImportTexturePointer((TEXTURES_FOLDER + std::to_string(App->resources->getResourceUuid("skybox_default_up", R_TEXTURE)) + DDS_EXTENSION).c_str());
		skybox_texs[DOWN] = (Texture*)App->importer->ImportTexturePointer((TEXTURES_FOLDER + std::to_string(App->resources->getResourceUuid("skybox_default_down", R_TEXTURE)) + DDS_EXTENSION).c_str());
		skybox_texs[FRONT] = (Texture*)App->importer->ImportTexturePointer((TEXTURES_FOLDER + std::to_string(App->resources->getResourceUuid("skybox_default_front", R_TEXTURE)) + DDS_EXTENSION).c_str());
		skybox_texs[BACK] = (Texture*)App->importer->ImportTexturePointer((TEXTURES_FOLDER + std::to_string(App->resources->getResourceUuid("skybox_default_back", R_TEXTURE)) + DDS_EXTENSION).c_str());
	}
	skybox->setAllTextures(skybox_texs);

	quadtree = new Quadtree(AABB(float3(-50, -10, -50), float3(50, 10, 50)));

	if (App->is_game && !App->debug_game && main_scene != 0)
	{
		LoadScene((SCENES_FOLDER + std::to_string(main_scene) + SCENE_EXTENSION).c_str());
	}
	LoadScene("Assets/Scenes/alitaRunning.scene");

	return true;
}

// Load assets
bool ModuleScene::CleanUp()
{
	for (auto it = game_objects.begin(); it != game_objects.end();it++)
		delete *it;

	game_objects.clear();
	game_objs_to_delete.clear();

	if(skybox) delete skybox;
	if(quadtree) delete quadtree;

	if (local_scene_save) {
		json_value_free(local_scene_save);
	}
	
	selected_obj.clear();
	return true;
}

update_status ModuleScene::PostUpdate(float dt)
{
	// Quadtree management
	if(draw_quadtree)
		quadtree->DebugDraw();

	if (quadtree_reload) {
		quadtree->Empty();
		for (auto it = game_objects.begin(); it != game_objects.end(); it++)
			if ((*it)->isStatic()) 
				quadtree->Insert(*it);

		quadtree_reload = false;
	}
	


	for (auto it = game_objs_to_delete.begin(); it != game_objs_to_delete.end(); it++)
	{
		//If something is deleted, ask quadtree to reload
		GameObject* current = (*it);
		quadtree_reload = true;
		if (selected_obj.size() && current == *selected_obj.begin()) 
			selected_obj.clear();
		game_objects.remove(current);

		// Remove child from parent
		if (GameObject* parent = (current)->getParent())
			parent->removeChild(current);

		// Set parent of the children nullptr, they are all going to die
		std::list<GameObject*> children;
		current->getChildren(children);
		for (auto it = children.begin(); it != children.end(); it++)
			(*it)->setParent(nullptr);

		delete *it;
	}

	game_objs_to_delete.clear();
	ManageSceneSaveLoad();

	return UPDATE_CONTINUE;
}

// Update
update_status ModuleScene::Update(float dt)
{

	if (App->input->GetKey(SDL_SCANCODE_K) == KEY_DOWN)
	{
		GameObject* obj = new GameObject("TEST");

		obj->own_half_size = float3(0.5, 0.5, 0.5);

		OBB* obb = ((ComponentAABB*)obj->getComponent(C_AABB))->getOBB();

		obb->SetFrom(AABB(float3(-0.5, -0.5, -0.5), float3(0.5, 0.5, 0.5)));

		obj->addComponent(COLLIDER_CUBE);

	}

	if (!ImGui::IsMouseHoveringAnyWindow() && App->input->GetMouseButton(1) == KEY_DOWN && !ImGuizmo::IsOver() && App->camera->selected_camera == App->camera->background_camera)
	{
		float x = (((App->input->GetMouseX() / (float)App->window->main_window->width) * 2) - 1);
		float y = (((((float)App->window->main_window->height - (float)App->input->GetMouseY()) / (float)App->window->main_window->height) * 2) - 1);

		
		GameObject* picked = MousePicking();
		if (picked != nullptr) {
			if (!App->input->GetKey(SDL_SCANCODE_LCTRL)) {
				App->scene->selected_obj.clear();
			}
			selected_obj.push_back(picked);
		}
		else {
			App->scene->selected_obj.clear();
		}
	}

	for (auto it = game_objects.begin(); it != game_objects.end(); it++)
		(*it)->Update(App->time->getGameDeltaTime()/1000);

	if (!audiolistenerdefault && App->input->GetKey(SDL_SCANCODE_X) == KEY_DOWN)
	{
		// Create audio listener
		audiolistenerdefault = new GameObject("Default Audio Listener");
		audiolistenerdefault->addComponent(AUDIOLISTENER);
	}

	if (App->input->GetKey(SDL_SCANCODE_Z) == KEY_DOWN)
	{
		// Create audio source
		GameObject* audiosource = new GameObject("Audio Source");
		ComponentAudioSource* component = (ComponentAudioSource*)audiosource->addComponent(AUDIOSOURCE);
		component->SetSoundID(AK::EVENTS::PUNCH);
		component->SetSoundName("Punch");
		component = (ComponentAudioSource*)audiosource->addComponent(AUDIOSOURCE);
		component->SetSoundID(AK::EVENTS::FOOTSTEPS);
		component->SetSoundName("Footsteps");
	}

	//App->physics->UpdatePhysics();

	return UPDATE_CONTINUE;
}

void ModuleScene::DrawScene(float3 camera_pos)
{
	if (skybox)
	{
		skybox->updatePosition(camera_pos);
		skybox->Draw();
	}

	App->debug->DrawShapes();

	std::list<GameObject*> drawable_gameobjects;

	for (auto it = game_objects.begin(); it != game_objects.end(); it++) {
		if (!(*it)->isStatic())
			drawable_gameobjects.push_back(*it);
	}

	if(App->camera->override_editor_cam_culling){
		quadtree_checks = quadtree->Intersect(drawable_gameobjects, *App->camera->override_editor_cam_culling->getFrustum());
	}
	else{
		quadtree_checks = 0;
		for (auto it = game_objects.begin(); it != game_objects.end(); it++) {
			if ((*it)->isStatic())
				drawable_gameobjects.push_back(*it);
		}
	}
	
	for (auto it = drawable_gameobjects.begin(); it != drawable_gameobjects.end(); it++)
		(*it)->Draw();

	quadtree_ignored_obj = game_objects.size() - drawable_gameobjects.size();
}

bool sortCloserRayhit(const RayHit& a, const RayHit& b) { return a.distance < b.distance; }

GameObject* ModuleScene::MousePicking(GameObject* ignore)
{
	float x = (((App->input->GetMouseX() / (float)App->window->main_window->width) * 2) - 1);
	float y = (((((float)App->window->main_window->height - (float)App->input->GetMouseY()) / (float)App->window->main_window->height) * 2) - 1);

	GameObject* ret = nullptr;

	Frustum* f = App->camera->selected_camera->getFrustum();
	Ray ray = f->UnProjectLineSegment(x , y).ToRay();

	std::list<GameObject*> intersected_objs;

	for (auto it = game_objects.begin(); it != game_objects.end(); it++)
	{
		if ((*it)->getComponent(MESH))
		{
			OBB* obb = ((ComponentAABB*)(*it)->getComponent(C_AABB))->getOBB();
			if (ray.Intersects(*obb) && *it != ignore)
				intersected_objs.push_back(*it);
		}
	}
	
	if (intersected_objs.empty())
		return ret;

	std::list<RayHit> ray_hits;
	
	for (auto it = intersected_objs.begin(); it != intersected_objs.end(); it++)
	{
		Transform* global = ((ComponentTransform*)(*it)->getComponent(TRANSFORM))->global;
		std::list<Component*> meshes;
		(*it)->getComponents(MESH, meshes);
		for (auto it2 = meshes.begin(); it2 != meshes.end(); it2++)
		{
			Mesh* mesh = ((ComponentMesh*)*it2)->getMesh();

			for (int i = 0; i < mesh->getNumTris(); i++)
			{
				const float3* vertices = mesh->getVertices();
				const Tri* tris = mesh->getTris();

				Triangle t((global->getRotation() * (global->getScale().Mul(vertices[tris[i].v1])) + global->getPosition()),
					(global->getRotation() * (global->getScale().Mul(vertices[tris[i].v2])) + global->getPosition()),
					(global->getRotation() * (global->getScale().Mul(vertices[tris[i].v3])) + global->getPosition()));

				RayHit hit(*it);

				if (ray.Intersects(t, &hit.distance, &hit.intersection_point))
					ray_hits.push_back(hit);
			}
		}
	}

	if (ray_hits.empty())
		return ret;
	else {
		ray_hits.sort(sortCloserRayhit);
		return ray_hits.front().obj;
	}
}


float3 ModuleScene::MousePickingHit(GameObject* ignore)
{

	float x = (((App->input->GetMouseX() / (float)App->window->main_window->width) * 2) - 1);
	float y = (((((float)App->window->main_window->height - (float)App->input->GetMouseY()) / (float)App->window->main_window->height) * 2) - 1);

	GameObject* ret = nullptr;

	Frustum* f = App->camera->selected_camera->getFrustum();
	Ray ray = f->UnProjectLineSegment(x, y).ToRay();

	std::list<GameObject*> intersected_objs;

	for (auto it = game_objects.begin(); it != game_objects.end(); it++)
	{
		if ((*it)->getComponent(MESH))
		{
			OBB* obb = ((ComponentAABB*)(*it)->getComponent(C_AABB))->getOBB();
			if (ray.Intersects(*obb) && *it != ignore)
				intersected_objs.push_back(*it);
		}
	}

	if (intersected_objs.empty())
		return float3::zero;

	std::list<RayHit> ray_hits;

	for (auto it = intersected_objs.begin(); it != intersected_objs.end(); it++)
	{
		Transform* global = ((ComponentTransform*)(*it)->getComponent(TRANSFORM))->global;
		std::list<Component*> meshes;
		(*it)->getComponents(MESH, meshes);
		for (auto it2 = meshes.begin(); it2 != meshes.end(); it2++)
		{
			Mesh* mesh = ((ComponentMesh*)*it2)->getMesh();

			for (int i = 0; i < mesh->getNumTris(); i++)
			{
				const float3* vertices = mesh->getVertices();
				const Tri* tris = mesh->getTris();

				Triangle t((global->getRotation() * (global->getScale().Mul(vertices[tris[i].v1])) + global->getPosition()),
					(global->getRotation() * (global->getScale().Mul(vertices[tris[i].v2])) + global->getPosition()),
					(global->getRotation() * (global->getScale().Mul(vertices[tris[i].v3])) + global->getPosition()));

				RayHit hit(*it);

				if (ray.Intersects(t, &hit.distance, &hit.intersection_point))
					ray_hits.push_back(hit);
			}
		}
	}

	if (ray_hits.empty())
		return float3::zero;
	else {
		ray_hits.sort(sortCloserRayhit);
		return ray_hits.front().intersection_point;
	}
}

GameObject* ModuleScene::duplicateGameObject(GameObject * gobj) {

	if (!gobj)
		return nullptr;
	// Duplicate go
	JSON_Value* go_deff = json_value_init_object();
	gobj->Save(json_object(go_deff));
	GameObject* duplicated_go = new GameObject(json_object(go_deff));
	App->scene->addGameObject(duplicated_go);
	json_value_free(go_deff);

	// Duplicate children
	std::list<GameObject*> children;
	gobj->getChildren(children);
	
	for (auto it = children.begin(); it != children.end(); it++) {
		// Duplicate child
		GameObject* curr_child = *it;
		GameObject* duplicated_child = duplicateGameObject(curr_child);
		duplicated_child->setParent(duplicated_go);
		duplicated_go->addChild(duplicated_child);
	}

	return duplicated_go;
}

GameObject* ModuleScene::getGameObject(uint uuid) const
{
	for (auto it = game_objects.begin(); it != game_objects.end(); it++)
		if (uuid == (*it)->getUUID())
			return *it;

	return nullptr;
}
std::list<GameObject*> ModuleScene::getGameObjectsByTag(std::string tag) {

	std::list<GameObject*> ret;
	for (auto it = game_objects.begin(); it != game_objects.end(); it++)
		if (tag == (*it)->tag)
			ret.push_back((*it));

	return ret;
}


void ModuleScene::getGameObjectsByComponent(Component_type type, std::list<GameObject*>& list_to_fill)
{
	for (auto it = game_objects.begin(); it != game_objects.end(); it++) {
		if ((*it)->getComponent(type) != nullptr) {
			list_to_fill.push_back(*it);
		}
	}
}


void ModuleScene::ClearScene()
{
	for (auto it = game_objects.begin(); it != game_objects.end(); it++)
		delete *it;

	game_objects.clear();

	selected_obj.clear();
}

void ModuleScene::getRootObjs(std::list<GameObject*>& list_to_fill)
{
	for (auto it = game_objects.begin(); it != game_objects.end(); it++)
		if (!(*it)->getParent())
			list_to_fill.push_back(*it);

	for (auto it = game_objs_to_delete.begin(); it != game_objs_to_delete.end(); it++) {
		bool iterator_found = false;
		for (auto it2 = list_to_fill.begin(); it2 != list_to_fill.end(); it2++) {
			if (*it == *it2){
				iterator_found = true;
				break;
			}
		}
		list_to_fill.remove(*it);
	}		
}

void ModuleScene::CleanScriptComponents()
{
	for (auto it = game_objects.begin(); it != game_objects.end(); it++) {
		std::list<Component*> components;
		(*it)->getComponents(components);

		for (auto it = components.begin(); it != components.end(); it++) {
			if ((*it)->getType() == SCRIPT) {
				((ComponentScript*)(*it))->CleanUp();
			}
		}
	}
}

void ModuleScene::LoadScriptComponents() {
	for (auto it = game_objects.begin(); it != game_objects.end(); it++) {
		std::list<Component*> components;
		(*it)->getComponents(components);

		for (auto it = components.begin(); it != components.end(); it++) {
			if ((*it)->getType() == SCRIPT) {
				((ComponentScript*)(*it))->LoadResource();
			}
		}
	}
}

GameObject* ModuleScene::getCanvasGameObject()
{
	std::list<GameObject*> GOs = std::list<GameObject*>();
	getGameObjectsByComponent(Component_type::CANVAS, GOs);
	if (GOs.empty()) { //check if canvas already exists
		GameObject* canvas = new GameObject("Canvas",nullptr, true);
		canvas->addComponent(Component_type::CANVAS);
		return canvas;
	}
	else {
		return *GOs.begin();
	}
	
}



void ModuleScene::deleteGameObjectRecursive(GameObject* gobj)
{
	game_objs_to_delete.push_back(gobj);

	std::list<GameObject*> children;
	gobj->getChildren(children);

	for (auto it = children.begin(); it != children.end(); it++)
		deleteGameObjectRecursive(*it);
}

void ModuleScene::AskPrefabLoadFile(const char * path, float3 pos, float3 eulerang) {
	want_load_prefab_file = true;
	prefab_load_spawn.setPosition(pos);
	prefab_load_spawn.setRotationEuler(eulerang);
	path_to_load_prefab = path;

}

void ModuleScene::AskSceneSaveFile(char * scene_name) {
	want_save_scene_file = true;
	scene_to_save_name = scene_name;
}

void ModuleScene::AskSceneLoadFile(char * path) {
	want_load_scene_file = true;
	path_to_load_scene = path;
}


void ModuleScene::ManageSceneSaveLoad() {
	if (want_save_scene_file) {
		SaveScene(scene_to_save_name);
		want_save_scene_file = false;
	}
	if (want_load_scene_file) {
		LoadScene(path_to_load_scene.c_str());
		want_load_scene_file = false;
	}
	if (want_load_prefab_file) {
		LoadPrefab(path_to_load_prefab.c_str());
		want_load_prefab_file = false;
	}
	if (want_local_save) {
		local_scene_save = serializeScene();
		want_local_save = false;
	}
	if (want_local_load) {
		if (local_scene_save) {
			ClearScene();
			loadSerializedScene(local_scene_save);
			json_value_free(local_scene_save);
			local_scene_save = nullptr;
		}
		else
			app_log->AddLog("Couldn't load locally serialized scene, value was NULL");

		want_local_load = false;
	}
}

void ModuleScene::SaveScene(std::string name) {

	if (App->fs.ExistisFile(name.c_str(), ASSETS_SCENES, SCENE_EXTENSION))
		app_log->AddLog("%s scene already created, overwritting...", name.c_str());

	App->fs.CreateEmptyFile(name.c_str(), ASSETS_SCENES, SCENE_EXTENSION);

	current_working_scene = name;
	working_on_existing_scene = true;

	JSON_Value* scene = serializeScene();

	std::string path;
	App->fs.FormFullPath(path, name.c_str(), ASSETS_SCENES, SCENE_EXTENSION);
	json_serialize_to_file_pretty(scene, path.c_str());

	// Free everything
	json_value_free(scene);
}


void ModuleScene::SavePrefab(GameObject* root, const char* name)
{
	if (App->fs.ExistisFile(name, ASSETS_PREFABS, PREFAB_EXTENSION))
		app_log->AddLog("%s scene already created, overwritting...", name);

	App->fs.CreateEmptyFile(name, ASSETS_PREFABS, PREFAB_EXTENSION);

	JSON_Value* prefab = serializePrefab(root);

	std::string path;
	App->fs.FormFullPath(path, name, ASSETS_PREFABS, PREFAB_EXTENSION);
	json_serialize_to_file_pretty(prefab, path.c_str());

	// Free everything
	json_value_free(prefab);
}

void ModuleScene::LoadScene(const char* path) {
	JSON_Value* scene = json_parse_file(path);
	if (!scene) {
		app_log->AddLog("Couldn't load %s, no value", path);
		return;
	}

	// Store current working scene
	std::string name = path;
	App->fs.getFileNameFromPath(name);
	current_working_scene = name;
	working_on_existing_scene = true;
	ClearScene();
	quadtree_reload = true;
	loadSerializedScene(scene);
	json_value_free(scene);
}

void ModuleScene::LoadPrefab(const char * path) {
	JSON_Value* prefab = json_parse_file(path);
	if (!prefab) {
		app_log->AddLog("Couldn't load %s, no value", path);
		return;
	}

	loadSerializedPrefab(prefab);
	json_value_free(prefab);

}

JSON_Value * ModuleScene::serializeScene() {

	JSON_Value* scene = json_value_init_object();	// Full file object
	JSON_Value* objects_array = json_value_init_array();	// Array of objects in the scene

	json_object_set_value(json_object(scene), "Game Objects", objects_array); // Add array to file

	for (auto it = game_objects.begin(); it != game_objects.end(); it++) {
		JSON_Value* object = json_value_init_object();	// Object in the array
		(*it)->Save(json_object(object));				// Fill content
		json_array_append_value(json_array(objects_array), object); // Add object to array
	}

	// TODO store editor camera(?)

	return scene;
}

JSON_Value* ModuleScene::serializePrefab(GameObject* root_obj)
{
	JSON_Value* prefab = json_value_init_object();	// Full file object
	JSON_Value* objects_array = json_value_init_array();	// Array of childs of the obj

	json_object_set_value(json_object(prefab), "Game Objects", objects_array); // Add array to file

	std::list<GameObject*> objects_to_save;
	root_obj->getAllDescendants(objects_to_save);

	objects_to_save.push_front(root_obj);

	for (auto it = objects_to_save.begin(); it != objects_to_save.end(); it++) {
		JSON_Value* object = json_value_init_object();	// Object in the array
		(*it)->Save(json_object(object));				// Fill content
		json_array_append_value(json_array(objects_array), object); // Add object to array
	}

	// TODO store editor camera(?)

	return prefab;
}

void ModuleScene::loadSerializedScene(JSON_Value * scene) {

	JSON_Array* objects = json_object_get_array(json_object(scene), "Game Objects");

	std::map<uint, GameObject*> loaded_gameobjects; // Store objects with parenting uuid
	std::map<uint, uint> child_parent;	// Store parenting info

	// Load all the objects, put them in the "loaded_gameobjects" array and set their parenting
	for (int i = 0; i < json_array_get_count(objects); i++) {
		JSON_Object* obj_deff = json_array_get_object(objects, i);
		uint parent = json_object_get_number(obj_deff, "Parent");  // Put the UUID of the parent in the same position as the child
		uint obj_uuid = json_object_get_number(obj_deff, "UUID");
		GameObject* obj = new GameObject(obj_deff);
		child_parent[obj_uuid] = parent;
		loaded_gameobjects[obj_uuid] = obj;
		game_objects.push_back(obj);
	}


	for (auto it = child_parent.begin(); it != child_parent.end(); it++) {
		uint obj_uuid = (*it).first;
		uint parent = (*it).second;
		if (parent != 0) {			// If the UUID of the parent is 0, it means that it has no parent
			// Look for the parent and child among gameobjects
			GameObject* parent_obj = loaded_gameobjects[parent];
			GameObject* child_obj = loaded_gameobjects[obj_uuid];
			// Link parent and child
			if(parent_obj){
				child_obj->setParent(parent_obj);
				parent_obj->addChild(child_obj);
			}
		}
	}

	// Push all gameobjects with handled parenting in the scene
	/*for (auto it = loaded_gameobjects.begin(); it != loaded_gameobjects.end(); it++) {
		game_objects.push_back((*it).second);			
	}*/
}

void ModuleScene::loadSerializedPrefab(JSON_Value * prefab) {

	JSON_Array* objects = json_object_get_array(json_object(prefab), "Game Objects");

	std::map<uint, GameObject*> loaded_gameobjects; // Store objects with parenting uuid
	std::map<uint, uint> child_parent;	// Store parenting info

	// Load all the objects, put them in the "loaded_gameobjects" array and set their parenting
	for (int i = 0; i < json_array_get_count(objects); i++) {
		JSON_Object* obj_deff = json_array_get_object(objects, i);
		uint parent = json_object_get_number(obj_deff, "Parent");  // Put the UUID of the parent in the same position as the child
		uint obj_uuid = json_object_get_number(obj_deff, "UUID");
		GameObject* obj = new GameObject(obj_deff);
		child_parent[obj_uuid] = parent;
		loaded_gameobjects[obj_uuid] = obj;
		game_objects.push_back(obj);
	}

	GameObject* father_of_all = nullptr; //Store this for setting prefab spawn position

	for (auto it = child_parent.begin(); it != child_parent.end(); it++) {
		uint obj_uuid = (*it).first;
		uint parent = (*it).second;
		if (parent != 0) {			// If the UUID of the parent is 0, it means that it has no parent
			// Look for the parent and child among gameobjects
			GameObject* parent_obj = loaded_gameobjects[parent];
			GameObject* child_obj = loaded_gameobjects[obj_uuid];
			// Link parent and child
			if (parent_obj) {
				child_obj->setParent(parent_obj);
				parent_obj->addChild(child_obj);
			}
		}
		else { // This is the root object of the prefab
			father_of_all = loaded_gameobjects[obj_uuid];
		}
	}

	// Set the spawn position of the prefab
	ComponentTransform* c_trans = (ComponentTransform*)father_of_all->getComponent(TRANSFORM);
	*c_trans->local = prefab_load_spawn;

	// Push all gameobjects with handled parenting in the scene
	/*for (auto it = loaded_gameobjects.begin(); it != loaded_gameobjects.end(); it++) {
		game_objects.push_back((*it).second);
	}*/
}





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
#include "ComponentRectTransform.h"
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
#include <filesystem>

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
	//LoadScene("Assets/Scenes/audio.scene");

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
	
	std::experimental::filesystem::remove_all(USER_AUTOSAVES_FOLDER); // Remove AutoSaves folder

	return true;
}

update_status ModuleScene::PostUpdate(float dt)
{
	// Quadtree management
	if(draw_quadtree && !App->is_game)
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
		if (!selected_obj.empty() && current == *selected_obj.begin()) 
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

		obj->addComponent(PHYSICS);

	}

	if (App->input->GetKey(SDL_SCANCODE_J) == KEY_DOWN)
	{
		GameObject* obj = new GameObject("TEST_TRIGGERS");

		obj->addComponent(TRIGGER);

	}


	for (auto it = game_objects.begin(); it != game_objects.end(); it++)
		(*it)->Update(App->time->getGameDeltaTime()/1000);

	//if (!audiolistenerdefault && App->input->GetKey(SDL_SCANCODE_X) == KEY_DOWN)
	//{
	//	// Create audio listener
	//	audiolistenerdefault = new GameObject("Default Audio Listener");
	//	audiolistenerdefault->addComponent(AUDIOLISTENER);
	//}

	//if (App->input->GetKey(SDL_SCANCODE_Z) == KEY_DOWN)
	//{
	//	// Create audio source
	//	GameObject* audiosource = new GameObject("Audio Source");
	//	ComponentAudioSource* component = (ComponentAudioSource*)audiosource->addComponent(AUDIOSOURCE);
	//	component->SetSoundID(AK::EVENTS::PUNCH);
	//	component->SetSoundName("Punch");
	//	component = (ComponentAudioSource*)audiosource->addComponent(AUDIOSOURCE);
	//	component->SetSoundID(AK::EVENTS::FOOTSTEPS);
	//	component->SetSoundName("Footsteps");
	//}

	if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT && App->input->GetKey(SDL_SCANCODE_Z) == KEY_DOWN)
	{
		UndoScene();
	}

	if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT && App->input->GetKey(SDL_SCANCODE_Y) == KEY_DOWN)
	{
		RedoScene();
	}

	if (App->input->GetKey(SDL_SCANCODE_C) == KEY_DOWN)
	{
		want_autosave = true;
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

	if (!App->is_game)
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


void ModuleScene::DrawInGameUI()
{
	GameObject* canvas = getCanvasGameObject();
	if (canvas != nullptr)
	{
		bool light = glIsEnabled(GL_LIGHTING);

		glDisable(GL_LIGHTING);

		ComponentRectTransform* rectTransform = (ComponentRectTransform*)canvas->getComponent(Component_type::RECTTRANSFORM);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		float left = rectTransform->getGlobalPos().x;
		float right = rectTransform->getGlobalPos().x + rectTransform->getWidth();
		float top = rectTransform->getGlobalPos().y + rectTransform->getHeight();
		float bottom = rectTransform->getGlobalPos().y;

		/*float left = rectTransform->GetGlobalPos().x;
		float right = rectTransform->GetGlobalPos().x + rectTransform->GetWidth();
		float top = rectTransform->GetGlobalPos().y + rectTransform->GetHeight();
		float bottom = rectTransform->GetGlobalPos().y;*/
		float zNear = -10.f;
		float zFar = 10.f;
		float3 min = { left, bottom, zNear };
		float3 max = { right, top, zFar };

		ui_render_box.minPoint = min;
		ui_render_box.maxPoint = max;

		glOrtho(left, right, bottom, top, zNear, zFar);
		float3 corners[8];
		ui_render_box.GetCornerPoints(corners);
		App->renderer3D->DrawDirectAABB(ui_render_box);
		/*glBegin(GL_TRIANGLES);
		glVertex2f(-1, -1);
		glVertex2f(1, -1);
		glVertex2f(1, 1);
		glEnd();*/

		std::list<GameObject*> UIGameObjects = getUIGameObjects(); // first draw UI components
		for (auto it : UIGameObjects)
		{
			it->Draw();
		}

		if (light) {
			glEnable(GL_LIGHTING);
		}
	}
}

bool sortCloserRayhit(const RayHit& a, const RayHit& b) { return a.distance < b.distance; }

GameObject* ModuleScene::MousePicking(float x, float y, GameObject* ignore)
{
	GameObject* ret = nullptr;

	Frustum* f = App->camera->editor_camera->getFrustum();
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


float3 ModuleScene::MousePickingHit(float x, float y, GameObject* ignore)
{
	if (!App->camera->game_camera)
		return float3::zero;

	GameObject* ret = nullptr;

	Frustum* f = App->camera->game_camera->getFrustum();
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
	prev_selected_obj.clear();
	for (auto it = selected_obj.begin(); it != selected_obj.end(); it++)
		prev_selected_obj.push_back((*it)->getUUID());

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

std::list<GameObject*> ModuleScene::getUIGameObjects()
{
	std::list<GameObject*>UIGameObjects;
	for (auto it : game_objects)
	{
		if (it->is_UI)
			UIGameObjects.push_back(it);
	}
	return UIGameObjects;
}

std::list<GameObject*> ModuleScene::getGameObjectWithButton()
{
	std::list<GameObject*>uiGOs = getUIGameObjects();
	std::list<GameObject*>buttonsObjs;
	for (auto it : uiGOs)
	{
		if (it->getComponent(Component_type::UI_BUTTON))
		{
			buttonsObjs.push_back(it);
		}
	}
	return buttonsObjs;
	
}

GameObject* ModuleScene::getCanvasGameObject(bool createCanvas)
{
	std::list<GameObject*> GOs = std::list<GameObject*>();
	getGameObjectsByComponent(Component_type::CANVAS, GOs);
	if (GOs.empty() && createCanvas) { //check if canvas already exists
		GameObject* canvas = new GameObject("Canvas",nullptr, true);
		canvas->addComponent(Component_type::CANVAS);
		return canvas;
	}
	else if (GOs.empty())
	{
		return nullptr;
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

void ModuleScene::AskPrefabLoadFile(PrefabData data) {
	prefabs_to_spawn.push_back(data);
}

void ModuleScene::AskSceneSaveFile(char * scene_name) {
	want_save_scene_file = true;
	scene_to_save_name = scene_name;
}

void ModuleScene::AskSceneLoadFile(const char * path) {
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
	if (want_autosave) {
		AutoSaveScene();
		want_autosave = false;
	}
	if (prefabs_to_spawn.size() > 0) {

		for(auto it = prefabs_to_spawn.begin(); it != prefabs_to_spawn.end(); it++)
			LoadPrefab((*it));

		prefabs_to_spawn.clear();
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

	for (auto it = prev_selected_obj.begin(); it != prev_selected_obj.end(); it++)
	{
		GameObject* go = getGameObject((*it));
		if (go)
			selected_obj.push_back(go);
	}
}

void ModuleScene::LoadPrefab(PrefabData data) {
	JSON_Value* prefab = json_parse_file(data.file.c_str());
	if (!prefab) {
		app_log->AddLog("Couldn't load %s, no value", data.file.c_str());
		return;
	}

	GameObject* go = loadSerializedPrefab(prefab);

	// Set transform
	Transform trans;
	trans.setPosition(data.pos);
	trans.setRotationEuler(data.euler);
	ComponentTransform* c_trans = (ComponentTransform*)go->getComponent(TRANSFORM);
	*c_trans->local = trans;

	// Force uuid
	//go->forceUUID(data.forced_uuid);

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

GameObject* ModuleScene::loadSerializedPrefab(JSON_Value * prefab) {

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

	return father_of_all;
	// Set the spawn position of the prefab

	// Push all gameobjects with handled parenting in the scene
	/*for (auto it = loaded_gameobjects.begin(); it != loaded_gameobjects.end(); it++) {
		game_objects.push_back((*it).second);
	}*/
}

void ModuleScene::AutoSaveScene()
{
	// Save current scene with a UUID as name
	std::string name = std::to_string(random32bits());
	App->fs.CreateEmptyFile(name.c_str(), ASSETS_AUTOSAVES, SCENE_EXTENSION);
	JSON_Value* scene = serializeScene();

	std::string path;
	App->fs.FormFullPath(path, name.c_str(), ASSETS_AUTOSAVES, SCENE_EXTENSION);
	json_serialize_to_file_pretty(scene, path.c_str());
	json_value_free(scene);

	// If there are too many autosaves in the undo remove the oldest
	undo_list.push_front(path);
	if (undo_list.size() > MAX_AUTOSAVES)
	{
		std::string to_destroy = undo_list.back();
		App->fs.getFileNameFromPath(to_destroy);
		App->fs.DestroyFile(to_destroy.c_str(), ASSETS_AUTOSAVES, SCENE_EXTENSION);
		undo_list.pop_back();
	}

	// Clear the redo list because a new action has been made
	if (!redo_list.empty())
		redo_list.clear();
}

void ModuleScene::UndoScene()
{
	if (undo_list.size() > 1)
	{
		// Load second scene autosave from undo list (the first one would be the actual)
		std::string path = (std::next(undo_list.begin()))->c_str();
		AskSceneLoadFile(path.c_str());
		// Send scene autosave to redo list
		redo_list.push_front(undo_list.front());
		undo_list.pop_front();
	}
}

void ModuleScene::RedoScene()
{
	if (redo_list.size() > 0)
	{
		// Load first scene autosave from redo list
		AskSceneLoadFile(redo_list.front().c_str());
		// Send scene autosave to undo list
		undo_list.push_front(redo_list.front());
		redo_list.pop_front();
	}
}

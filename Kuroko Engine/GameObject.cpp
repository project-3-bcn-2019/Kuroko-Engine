#include "GameObject.h"
#include "Random.h"
#include "ComponentMesh.h"
#include "ComponentTransform.h"
#include "ComponentAABB.h"
#include "ComponentCamera.h"
#include "ComponentScript.h"
#include "ComponentAudioListener.h"
#include "ComponentAudioSource.h"
#include "Camera.h"
#include "Application.h"
#include "ModuleUI.h"
#include "ModuleScene.h"
#include "ModuleCamera3D.h"
#include "Applog.h"
#include "ModuleRenderer3D.h"


GameObject::GameObject(const char* name, GameObject* parent) : name(name), parent(parent), id(App->scene->last_gobj_id++), uuid(random32bits()) 
{
	if (uuid == 0)		// Remote case, UUID can never be 0!
		uuid += 1;

	addComponent(TRANSFORM);
	addComponent(C_AABB);
	App->scene->addGameObject(this);
	
}

GameObject::GameObject(JSON_Object* deff): uuid(random32bits()) {
	name = json_object_get_string(deff, "name");
	is_static = json_object_get_boolean(deff, "static");

	// Create components
	JSON_Array* json_components = json_object_get_array(deff, "Components");

	for (int i = 0; i < json_array_get_count(json_components); i++) {
		JSON_Object* component_deff = json_array_get_object(json_components, i);
		std::string type;
		if(const char* type_c_str = json_object_get_string(component_deff, "type"))
			 type = type_c_str;
		Component* component = nullptr;
		if (type == "transform") {
			component = new ComponentTransform(component_deff, this);
		}
		else if (type == "AABB") {
			component = new ComponentAABB(this);
		}
		else if (type == "mesh") {
			component = new ComponentMesh(component_deff, this);
		}
		else if (type == "script") {
			component = new ComponentScript(component_deff, this);
		}
		else if (type == "audioListener") {
			component = new ComponentAudioListener(component_deff, this);
		}
		else if (type == "audioSource") {
			component = new ComponentAudioSource(component_deff, this);
		}

		// Set component's parent-child
		if (!component){
			app_log->AddLog("WARNING! Component of type %s could not be loaded", type.c_str());
			continue;
		}
		addComponent(component);
	}

	
}

GameObject::~GameObject()
{
	for (auto it = components.begin(); it != components.end(); it++)
		delete *it;

}



bool GameObject::Update(float dt)
{
	if (is_active)
	{
		for (auto it = components.begin(); it != components.end(); it++)
			if(!(*it)->Update(dt))
				{ app_log->AddLog("error in gameobject %s", name.c_str()); return false;}
	}

	return true;
}


void GameObject::Draw() const
{
	if (is_active)
	{
		for (auto it = components.begin(); it != components.end(); it++)
			(*it)->Draw();

		if (App->scene->selected_obj == this)
			DrawSelected();
	}
}


void GameObject::DrawSelected() const
{
	for (auto it = components.begin(); it != components.end(); it++)
	{
		if ((*it)->getType() == MESH)
			((ComponentMesh*)*it)->DrawSelected();
	}

	std::list<GameObject*> children;
	getChildren(children);

	for (auto it = children.begin(); it != children.end(); it++)
		(*it)->DrawSelected();
}

Component* GameObject::getComponent(Component_type type) const
{
	for (std::list<Component*>::const_iterator it = components.begin(); it != components.end(); it++)
	{
		if ((*it)->getType() == type)
			return *it;
	}

	return nullptr;
}

bool GameObject::getComponents(Component_type type, std::list<Component*>& list_to_fill) const
{
	for (std::list<Component*>::const_iterator it = components.begin(); it != components.end(); it++)
	{
		if ((*it)->getType() == type)
			list_to_fill.push_back(*it);
	}

	return !list_to_fill.empty();
}

GameObject* GameObject::getChild(const char* name) const
{
	for (std::list<GameObject*>::const_iterator it = children.begin(); it != children.end(); it++)
	{
		if ((*it)->getName() == name)
			return *it;
	}

	return nullptr;
}

void GameObject::getAllDescendants(std::list<GameObject*>& list_to_fill) const
{
	for (auto it = children.begin(); it != children.end(); it++)
		list_to_fill.push_back(*it);

	for (auto it = children.begin(); it != children.end(); it++)
		(*it)->getAllDescendants(list_to_fill);

	return;
}

Component* GameObject::addComponent(Component_type type)
{
	Component* new_component = nullptr;

	switch (type)
	{
	case MESH:		
		new_component = new ComponentMesh(this); 
		components.push_back(new_component);
		((ComponentAABB*)getComponent(C_AABB))->Reload();
		break;
	case C_AABB:	
		if (!getComponent(C_AABB))
		{
			new_component = new ComponentAABB(this);
			components.push_back(new_component);
		}
		break;
	case TRANSFORM:
		if (!getComponent(TRANSFORM))
		{
			new_component = new ComponentTransform(this);
			components.push_back(new_component);
		}
		break;
	case CAMERA:
		if (!getComponent(CAMERA))
		{
			Camera* camera = new Camera();
			new_component = new ComponentCamera(this, camera);
			components.push_back(new_component);
		}
		break;
	case SCRIPT:
		new_component = new ComponentScript(this);
		components.push_back(new_component);
		break;
	case AUDIOLISTENER:
		if (!getComponent(AUDIOLISTENER))
		{
			new_component = new ComponentAudioListener(this);
			components.push_back(new_component);
		}
		break;
	case AUDIOSOURCE:
		new_component = new ComponentAudioSource(this);
		components.push_back(new_component);
		break;
	default:
		break;
	}

	return new_component;
}

void GameObject::addComponent(Component* component)
{
	if (!component)
		return;

	switch (component->getType())
	{
	case MESH:	
		components.push_back(component);
		((ComponentAABB*)getComponent(C_AABB))->Reload();
		break;
	case C_AABB:
		if (!getComponent(C_AABB))
			components.push_back(component);
		break;
	case TRANSFORM:
		if (!getComponent(TRANSFORM))
			components.push_back(component);
		break;
	case CAMERA:
		if (!getComponent(CAMERA))
			components.push_back(component);
		break;
	case SCRIPT:
		components.push_back(component);
		break;
	case AUDIOLISTENER:
		if (!getComponent(AUDIOLISTENER))
			components.push_back(component);
		break;
	case AUDIOSOURCE:
		components.push_back(component);
		break;
	default:
		break;
	}
}


void GameObject::removeComponent(Component* component)
{
	for (std::list<Component*>::iterator it = components.begin(); it != components.end(); it++)
	{
		if (component->getType() == TRANSFORM || component->getType() == C_AABB)
			continue;
		else if (*it == component)
		{
			components.remove(component);

			switch (component->getType())
			{
			case MESH:
				((ComponentAABB*)getComponent(C_AABB))->Reload();
				break;
			case CAMERA:
				Camera* cam_to_remove = ((ComponentCamera*)component)->getCamera();
				App->camera->game_cameras.remove(cam_to_remove);
			}
			
			delete component;
			return;
		}
	}
	
}

void GameObject::Save(JSON_Object * config) {

	// Saving object own variables
	json_object_set_string(config, "name", name.c_str());
	json_object_set_boolean(config, "static", is_static);
	json_object_set_number(config, "UUID", uuid);
	if (parent) json_object_set_number(config, "Parent", parent->uuid);

	// Saving components components
	JSON_Value* component_array = json_value_init_array(); // Create array of components

	for (auto it = components.begin(); it != components.end(); it++) {
		JSON_Value* curr_component = json_value_init_object(); // Create new components 
		(*it)->Save(json_object(curr_component));			   // Save component
		json_array_append_value(json_array(component_array), curr_component); // Add them to array
	}

	json_object_set_value(config, "Components", component_array); // Save component array in object

}


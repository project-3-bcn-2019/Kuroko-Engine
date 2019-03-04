#include "GameObject.h"
#include "Random.h"
#include "ComponentMesh.h"
#include "ComponentRectTransform.h"
#include "ComponentTransform.h"
#include "ComponentAABB.h"
#include "ComponentCamera.h"
#include "ComponentCanvas.h"
#include "ComponentScript.h"
#include "ComponentAudioListener.h"
#include "ComponentAudioSource.h"
#include "ComponentImageUI.h"
#include "ComponentButtonUI.h"
#include "ComponentCheckBoxUI.h"
#include "ComponentTextUI.h"
#include "ComponentBone.h"
#include "ComponentBillboard.h"
#include "ComponentParticleEmitter.h"
#include "ComponentAnimation.h"
#include "ModulePhysics3D.h"
#include "ComponentAnimationEvent.h"
#include "ComponentColliderCube.h"

#include "Camera.h"
#include "Application.h"
#include "ModuleUI.h"
#include "ModuleScene.h"
#include "ModuleCamera3D.h"
#include "Applog.h"
#include "ModuleRenderer3D.h"



GameObject::GameObject(const char* name, GameObject* parent, bool UI) : name(name), parent(parent), id(App->scene->last_gobj_id++), uuid(random32bits())
{
	if (uuid == 0)		// Remote case, UUID can never be 0!
		uuid += 1;

	if (!UI) {
		addComponent(TRANSFORM);
		addComponent(C_AABB);
	}
	else { // for UI
		addComponent(RECTTRANSFORM);
		is_UI = true;
	}
	App->scene->addGameObject(this);
	
}


GameObject::GameObject(JSON_Object* deff): uuid(random32bits()) {
	name = json_object_get_string(deff, "name");
	tag = json_object_get_string(deff, "tag");
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
		else if (type == "bone") {
			component = new ComponentBone(component_deff, this);
		}
		else if (type == "animation") {
			component = new ComponentAnimation(component_deff, this);
		}
		else if (type == "camera") {
			component = new ComponentCamera(component_deff, this);
		}
		else if (type == "billboard") {
			component = new ComponentBillboard(component_deff, this);
		}
		else if (type == "particle_emitter") {
			component = new ComponentParticleEmitter(component_deff, this);
		}
		else if (type == "animation_event") {
			component = new ComponentAnimationEvent(component_deff, this);
		}

		else if (type == "collider_cube") {
			component = new ComponentColliderCube(component_deff, this);
		}
		// Set component's parent-child
		if (!component){
			app_log->AddLog("WARNING! Component of type %s could not be loaded", type.c_str());
			continue;
		}
		component->LoadCompUUID(component_deff);

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

		for (auto it = App->scene->selected_obj.begin(); it != App->scene->selected_obj.end(); it++) {
			if (*it == this) {
				DrawSelected();
				break;
			}
		}
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

Component * GameObject::getComponentByUUID(uint uuid) const {

	for (std::list<Component*>::const_iterator it = components.begin(); it != components.end(); it++) {
		if ((*it)->getUUID() == uuid)
			return *it;
	}

	return nullptr;
}

Component* GameObject::getChildComponent(uint uuid) const
{
	Component* component = nullptr;
	for (std::list<GameObject*>::const_iterator it = children.begin(); it != children.end(); ++it)
	{
		for (std::list<Component*>::const_iterator it_c = (*it)->components.begin(); it_c != (*it)->components.end(); it_c++)
		{
			if ((*it_c)->getUUID() == uuid)
			{
				component = (*it_c);
				break;
			}
		}
		if (component != nullptr)
			break;
		else
		{
			component = (*it)->getChildComponent(uuid);
		}
	}

	return component;
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

GameObject* GameObject::getChild(const char* name, bool  ignoreAssimpNodes) const
{
	GameObject* child = nullptr;

	for (std::list<GameObject*>::const_iterator it = children.begin(); it != children.end(); ++it)
	{
		if ((*it)->getName().find(name) != -1 && (!ignoreAssimpNodes || (*it)->getName().find("$AssimpFbx$") == std::string::npos))
		{
			child = (*it);
			break;
		}
		else
		{
			child = (*it)->getChild(name, ignoreAssimpNodes);
			if (child != nullptr)
				break;
		}
	}

	return child;
}

void GameObject::getAllDescendants(std::list<GameObject*>& list_to_fill) const
{
	for (auto it = children.begin(); it != children.end(); it++)
		list_to_fill.push_back(*it);

	for (auto it = children.begin(); it != children.end(); it++)
		(*it)->getAllDescendants(list_to_fill);

	return;
}

GameObject* GameObject::getAbsoluteParent()
{
	if (parent == nullptr)
		return this;
	else
		return parent->getAbsoluteParent();
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
	case CANVAS:
		if (!getComponent(CANVAS))
		{
			new_component = new ComponentCanvas(this);
			components.push_back(new_component);			
		}
		break;
	case RECTTRANSFORM:
		if (!getComponent(RECTTRANSFORM))
		{
			new_component = new ComponentRectTransform(this);
			components.push_back(new_component);
		}
		break;
	case UI_IMAGE:
		if (!getComponent(UI_IMAGE))
		{
			new_component = new ComponentImageUI(this);
			components.push_back(new_component);
		}
		break;
	case UI_BUTTON:
		if (!getComponent(UI_BUTTON))
		{
			new_component = new ComponentButtonUI(this);
			components.push_back(new_component);
		}
		break;	
	case UI_CHECKBOX:
		if (!getComponent(UI_CHECKBOX))
		{
			new_component = new ComponentCheckBoxUI(this);
			components.push_back(new_component);
		}
		break;
	case UI_TEXT:
		if (!getComponent(UI_TEXT))
		{
			new_component = new ComponentTextUI(this);
			components.push_back(new_component);
		}
		break;
	case BONE:
		new_component = new ComponentBone(this);
		components.push_back(new_component);
		break;
	case ANIMATION:
		if (!getComponent(ANIMATION))
		{
			new_component = new ComponentAnimation(this);
			components.push_back(new_component);
		}
		break;
	case COLLIDER_CUBE:
		if (!getComponent(COLLIDER_CUBE))
		{
			new_component = new ComponentColliderCube(this);
			components.push_back(new_component);
		}
		break;
	case BILLBOARD:
		new_component = new ComponentBillboard(this);
		components.push_back(new_component);
		break;
	case PARTICLE_EMITTER:
		new_component = new ComponentParticleEmitter(this);
		components.push_back(new_component);
		break;
	case ANIMATION_EVENT:
		new_component = new ComponentAnimationEvent(this);
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
	case BONE:
		components.push_back(component);
		break;
	case ANIMATION:
		if (!getComponent(ANIMATION))
			components.push_back(component);
		break;
	case BILLBOARD:
		components.push_back(component);
		break;
	case PARTICLE_EMITTER:
		components.push_back(component);
		break;
	case COLLIDER_CUBE:
		components.push_back(component);
		break;
	case ANIMATION_EVENT:
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
	json_object_set_string(config, "tag", tag.c_str());
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


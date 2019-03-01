#ifndef _GAME_OBJECT_
#define _GAME_OBJECT_
#include <list>
#include <string>
#include "Component.h"
#include "Globals.h"
#include "Parson\parson.h"

#include "MathGeoLib\Math\float3.h"


class GameObject
{
	friend class ComponentTransform;
	friend class ComponentAABB;
public:

	GameObject(const char* name, GameObject* parent = nullptr, bool UI = false );
	GameObject(JSON_Object* deff);
	~GameObject();

	bool Update(float dt);
	void Draw() const;
	void DrawSelected() const;
	
	uint getId() { return id; }
	uint getUUID() const{ return uuid; }

	void addComponent(Component* component);
	Component* addComponent(Component_type type);
	void removeComponent(Component* component);

	Component* getComponent(Component_type type) const;
	Component* getComponentByUUID(uint uuid) const;
	Component* getChildComponent(uint uuid) const;
	bool getComponents(Component_type type, std::list<Component*>& list_to_fill) const;
	void getComponents(std::list<Component*>& list_to_fill) const { list_to_fill = components; };

	GameObject* getChild(const char* name) const;
	void getChildren(std::list<GameObject*>& list_to_fill) const { list_to_fill = children; };
	void getAllDescendants(std::list<GameObject*>& list_to_fill) const;
	void addChild(GameObject* new_child) {if(new_child) children.push_back(new_child); };
	void removeChild(GameObject* child) { children.remove(child); }

	GameObject* getParent() const { return parent; };
	void setParent(GameObject* parent) { this->parent = parent; }// Can recieve nullptr

	float3 getCentroid() const { return centroid; };	// includes all childs
	float3 getHalfsize() const { return half_size; };	// includes all childs

	float3 getOwnCentroid() const { return own_centroid; };		// does not include childs
	float3 getOwnHalfsize() const { return own_half_size; };	// does not include childs

	bool isActive() const { return is_active; }
	bool isStatic() const { return is_static; }
	void setActive(bool state) { is_active = state; }
	void setStatic(bool state) { is_static = state; }

	void Rename(const char* new_name) { name = new_name; };
	std::string getName() const { return name; };

	void Save(JSON_Object* config);

	float3 own_centroid = float3::zero;		// does not include childs
	float3 own_half_size = float3::zero;	// does not include childs

private:

	std::list<Component*> components;
	std::list<GameObject*> children;
	std::string name;


	GameObject* parent = nullptr;

	float3 centroid = float3::zero;		// includes all childs
	float3 half_size = float3::zero;	// includes all childs



	const uint id = 0;
	// UUID can't be 0! Parents which UUID is 0, means that object has no parent.
	uint uuid = 0; 

public:

	bool is_active = true;
	bool is_static = false;
	bool is_UI = false;
	std::string tag = "undefined";

};
#endif
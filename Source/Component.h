#ifndef _COMPONENT_
#define _COMPONENT_


#define DELTA_ALPHA 0.01f

#include "Parson\parson.h"
#include "Globals.h"
#include "Random.h"
#include "MathGeoLib\Math\float3.h"
#include "Color.h"

class GameObject;

enum Component_type { NONE, MESH, TRANSFORM, C_AABB, CAMERA, SCRIPT, BONE, ANIMATION, CANVAS, RECTTRANSFORM,
UI_IMAGE, UI_CHECKBOX, UI_BUTTON, UI_TEXT, AUDIOLISTENER, AUDIOSOURCE, COLLIDER_CUBE, BILLBOARD, PARTICLE_EMITTER
};

class Component
{
public:

	Component() {};
	Component(GameObject* gameobject, Component_type type) : parent(gameobject), type(type), uuid(random32bits()) {};
	virtual ~Component() {};

	virtual bool Update(float dt) { return true; };
	virtual void Draw() const {};

	bool isActive() { return is_active; } const
	void setActive(bool state) { is_active = state; }

	Component_type getType() const { return type; };
	GameObject* getParent() const { return parent; };
	void setParent(GameObject* new_parent) { parent = new_parent; } // Can recieve nullptr
	uint getUUID() const { return uuid; }

	virtual void Save(JSON_Object* config) {}

	void SaveRange(JSON_Object* json, const char* name, const range<float>& range)
	{
		json_object_dotset_number(json, name, range.min);
		json_object_dotset_number(json, name, range.max);
	}

	void SaveColor(JSON_Object* json, const char* name, const Color& color)
	{
		json_object_dotset_number(json, name, color.r);
		json_object_dotset_number(json, name, color.g);
		json_object_dotset_number(json, name, color.b);
		json_object_dotset_number(json, name, color.a);
	}

	void SaveFloat3(JSON_Object* json, const char* name, const float3& vector)
	{
		json_object_dotset_number(json, name, vector.x);
		json_object_dotset_number(json, name, vector.y);
		json_object_dotset_number(json, name, vector.z);
	}

	range<float> LoadRange(JSON_Object* json, const char* name)
	{
		range<float> range;
		range.min = json_object_dotget_number(json, name);
		range.max = json_object_dotget_number(json, name);
		return range;
	}

	Color LoadColor(JSON_Object* json, const char* name)
	{
		Color color;
		color.r = json_object_dotget_number(json, name);
		color.g = json_object_dotget_number(json, name);
		color.b = json_object_dotget_number(json, name);
		color.a = json_object_dotget_number(json, name);
		return color;
	}

	float3 LoadFloat3(JSON_Object* json, const char* name)
	{
		float3 vector;
		vector.x = json_object_dotget_number(json, name);
		vector.y = json_object_dotget_number(json, name);
		vector.z = json_object_dotget_number(json, name);
		return vector;
	}

protected:

	bool is_active = true;
	const Component_type type = NONE;
	GameObject* parent = nullptr;
	uint uuid = 0;
};
#endif
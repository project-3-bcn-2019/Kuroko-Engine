#ifndef _COMPONENT_
#define _COMPONENT_


#define DELTA_ALPHA 0.01f

#include "Parson\parson.h"
#include "Globals.h"
#include "Random.h"
#include <vector>
#include <string>
#include <map>
#include "MathGeoLib\Math\float3.h"
#include "Color.h"

class GameObject;

enum Component_type { NONE, MESH, TRANSFORM, C_AABB, CAMERA, SCRIPT, BONE, ANIMATION, CANVAS, RECTTRANSFORM,
UI_IMAGE, UI_CHECKBOX, UI_BUTTON, UI_TEXT, UI_PROGRESSBAR, AUDIOLISTENER, AUDIOSOURCE, PHYSICS, TRIGGER, BILLBOARD, PARTICLE_EMITTER,
ANIMATION_EVENT, ANIMATOR
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
		json_object_dotset_number(json, (std::string(name) + "_min").c_str(), range.min);
		json_object_dotset_number(json, (std::string(name) + "_max").c_str(), range.max);
	}

	void SaveColor(JSON_Object* json, const char* name, const Color& color)
	{														  
		json_object_dotset_number(json, (std::string(name) + "_r").c_str(), color.r);
		json_object_dotset_number(json, (std::string(name) + "_g").c_str(), color.g);
		json_object_dotset_number(json, (std::string(name) + "_b").c_str(), color.b);
		json_object_dotset_number(json, (std::string(name) + "_a").c_str(), color.a);
	}

	void SaveFloat3(JSON_Object* json, const char* name, const float3& vector)
	{
		json_object_dotset_number(json, (std::string(name) + "_x").c_str(), vector.x);
		json_object_dotset_number(json, (std::string(name) + "_y").c_str(), vector.y);
		json_object_dotset_number(json, (std::string(name) + "_z").c_str(), vector.z);
	}									

	range<float> LoadRange(JSON_Object* json, const char* name)
	{
		range<float> range;
		range.min = json_object_dotget_number(json, (std::string(name) + "_min").c_str());
		range.max = json_object_dotget_number(json, (std::string(name) + "_max").c_str());
		return range;
	}

	Color LoadColor(JSON_Object* json, const char* name)
	{
		Color color;
		color.r = json_object_dotget_number(json, (std::string(name) + "_r").c_str());
		color.g = json_object_dotget_number(json, (std::string(name) + "_g").c_str());
		color.b = json_object_dotget_number(json, (std::string(name) + "_b").c_str());
		color.a = json_object_dotget_number(json, (std::string(name) + "_a").c_str());
		return color;
	}

	float3 LoadFloat3(JSON_Object* json, const char* name)
	{
		float3 vector;
		vector.x = json_object_dotget_number(json, (std::string(name) + "_x").c_str());
		vector.y = json_object_dotget_number(json, (std::string(name) + "_y").c_str());
		vector.z = json_object_dotget_number(json, (std::string(name) + "_z").c_str());
		return vector;
	}

protected:

	bool is_active = true;
	const Component_type type = NONE;
	GameObject* parent = nullptr;
	uint uuid = 0;

public:
	// For animation curr
	bool AnimSel = false;
	
	// Helper Functions
	std::string TypeToString();

	void SaveCommons(JSON_Object* config);
	void LoadCommons(JSON_Object* deff);

	// When Creating animations for components all of these functions
	// are required in order to process the events for the panel and 
	// for the actual play of animations for components
	// Each Animation event should have its own function, similar to a
	// callback system but by type

	// You have to create these function in each component, the virtual will
	// only be used when it fails to get animations from component and return
	// an invalid result.

	// Currently no values can be used in animations, only call function
	// certain keyframe, if really needed for vertical slice I will
	// implement it, but will take time and for each type of value used
	// a cast system should be added to every component so that it gets the
	// value wanted (similar to a union usage) maybe will create new union
	// based struct but for now should not be needed

	// You have to create each ComponentTypeAnimEVT enum, which holds the evt
	// types for the type of component you want to be animated
	// please don't make a case ERROR/NONE or put it with value -1
	// makes no sense as designers will not be able to actually go below or above
	// the event numbers

	virtual std::string EvTypetoString(int evt) { 
		//	Template for virtual EvTypetoString
		//	switch (evt)
		//	{
		//		case ComponentTypeAnimEVT::ANIMEVT1:
		//			return "ANIMEVT1";
		//		.
		//		.
		//		.
		//	}
		return "NO EVENTS"; 
	};
	
	virtual int getEvAmount() {	
		return -1 /*ComponentTypeAnimEVT::ANIMEVTMAX*/; 
	};

	virtual void ProcessAnimationEvents(std::map<int, void*>& evts) 
	{
		// Template for virtual AnimationEvent processing
		//	for(auto it_evt = evts.begin(); it_evt != evts.end(); ++it_evt)
		//	{
		//		switch(it->first)
		//		{
		//		case ComponentTypeAnimEVT::ANIMEVT1:
		//			CallFunctionDoEVT1();
		//		.
		//		.
		//		.
		//		}
		//	}
	};
};
#endif
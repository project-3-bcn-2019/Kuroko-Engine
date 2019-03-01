#ifndef _COMPONENT_ANIMATION
#define _COMPONENT_ANIMATION

#include "Component.h"
//TMP - substitute by Animation.h
#include "Globals.h"
#include <map>

//abomination
typedef std::map<int, void*> KeyframeVals;
typedef std::map<double, KeyframeVals> KeyMap;
typedef std::map<uint, std::map<double, std::map<int, void*>>> CompAnimMultiMap;
// Explanation
// std::multimap<uint,...> uint is the UUID of the component
// ...<...,std::map<...>> is the keyframes for the component
// ...<...,std::map<double,...>> uint is the frame (time) of keyframe
// ...<...,...<...,std::map<...,...>> is the keyframe info
// ...<...,...<...,std::map<int,...>> int is the component event triggered
// // ...<...,...<...,std::map<...,void*>> void* is the values required for the event to play


class ComponentAnimation : public Component
{
public:
	ComponentAnimation(GameObject* gameobject) : Component(gameobject, ANIMATION) {};   // empty constructor
	ComponentAnimation(JSON_Object* deff, GameObject* parent);
	~ComponentAnimation();

	bool Update(float dt);

	void ProcessComponentAnimations(float dt);

	uint getAnimationResource() const { return animation_resource_uuid; }
	void setAnimationResource(uint uuid);

	void Save(JSON_Object* config);

	bool Finished() const { return false; }

public:

	bool loop = false;
	float speed = 1.0f;

private:

	uint animation_resource_uuid = 0;

	float animTime = 0.0f;

	std::map<uint, uint> bones;

	// Component Animation
public:
	CompAnimMultiMap ComponentAnimations;
	int own_ticks = 0;
};

#endif // !_COMPONENT_ANIMATION
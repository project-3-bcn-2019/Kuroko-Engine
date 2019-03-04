#ifndef _ANIMATION_EVENT_
#define _ANIMATION_EVENT_

#include "Component.h"
#include "Parson/parson.h"
#include "Globals.h"
#include "Random.h"
#include <vector>
#include <string>
#include <map>
#include <list>

typedef std::map<int, void*> KeyframeVals;
typedef std::map<double, KeyframeVals> KeyMap;
typedef std::map<uint, std::map<double, std::map<int, void*>>> CompAnimMap;
// Explanation
// std::multimap<uint,...> uint is the UUID of the component
// ...<...,std::map<...>> is the keyframes for the component
// ...<...,std::map<double,...>> uint is the frame (time) of keyframe
// ...<...,...<...,std::map<...,...>> is the keyframe info
// ...<...,...<...,std::map<int,...>> int is the component event triggered
// // ...<...,...<...,std::map<...,void*>> void* is the values required for the event to play

struct AnimSet
{
	std::string name = "error";

	uint linked_animation = 0;

	CompAnimMap AnimEvts;
	int own_ticks = 0;
	int ticksXsecond = 0;
	bool loop = false;
	float speed = 1.0f;

	bool selected;
};

class ComponentAnimationEvent : public Component
{
public:
	ComponentAnimationEvent(GameObject* gameobject) : Component(gameobject, ANIMATION_EVENT) {} // empty constructor
	ComponentAnimationEvent(JSON_Object* deff, GameObject* parent);
	~ComponentAnimationEvent();

	bool Update(float dt);

	// Get the resource when there is

	void Play() { paused = false; }
	void Pause() { paused = true; }

	void Save(JSON_Object* config);

	bool Finished() const { return false; }
	bool isPaused() const { return paused; }
	
	void CheckLinkAnim();
private:

	float animTime = 0.0f;
	bool paused = false;

public:

	std::list<AnimSet> AnimEvts;
	AnimSet* curr = nullptr;

	
	
};
#endif
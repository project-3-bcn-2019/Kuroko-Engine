#ifndef _COMPONENT_BONE
#define _COMPONENT_BONE

#include "Component.h"
//TMP - substitute by Bone.h
#include "Globals.h"
#include "MathGeoLib/Math/float4x4.h"
#include <list>
/*ypedef std::map<int, void*> KeyframeVals;
typedef std::map<uint, KeyframeVals> KeyMap;
typedef std::map<double, std::map<uint, std::map<int, void*>>> CompAnimMap;
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

	bool selected;
};*/


class ComponentBone : public Component
{
public:
	ComponentBone(GameObject* gameobject) : Component(gameobject, BONE) {};   // empty constructor
	ComponentBone(JSON_Object* deff, GameObject* parent);
	~ComponentBone();

	void Save(JSON_Object* config);
	uint getBoneResource() { return bone_resource_uuid; }

	bool Update(float dt);

private:

	uint bone_resource_uuid = 0;
	float4x4 globalOffset = float4x4::identity;

private:
	float animTime = 0.0f;
	bool paused = false;

	int last_frame = -1;

public:
	/*std::list<AnimSet> AnimSets;
	AnimSet* curr = nullptr;*/
};

#endif // !_COMPONENT_BONE
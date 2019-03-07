#ifndef _COMPONENT_BONE
#define _COMPONENT_BONE

#include "Component.h"
//TMP - substitute by Bone.h
#include "Globals.h"
#include "MathGeoLib/Math/float4x4.h"
#include <list>

struct AnimSetB
{
	uint linked_animation = 0;

	std::map<double, std::map<uint, std::map<int, void*>>> AnimEvts;

	bool selected = false;
};


class ComponentBone : public Component
{
public:
	ComponentBone(GameObject* gameobject) : Component(gameobject, BONE) {};   // empty constructor
	ComponentBone(JSON_Object* deff, GameObject* parent);
	~ComponentBone();

	void Save(JSON_Object* config);
	uint getBoneResource() { return bone_resource_uuid; }

	bool Update(float dt);
	void ProcessCompAnimations(const uint anim_uuid, const int frame);

private:
	uint bone_resource_uuid = 0;
	float4x4 globalOffset = float4x4::identity;

private:
	float animTime = 0.0f;
	bool paused = false;

	int last_frame = -1;

public:
	std::map<uint,AnimSetB> AnimSets;
	AnimSetB* curr = nullptr;
};

#endif // !_COMPONENT_BONE
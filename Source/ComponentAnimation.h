#ifndef _COMPONENT_ANIMATION
#define _COMPONENT_ANIMATION

#include "Component.h"
//TMP - substitute by Animation.h
#include "Globals.h"
#include "ComponentBone.h"

class ComponentAnimation : public Component
{
public:
	ComponentAnimation(GameObject* gameobject) : Component(gameobject, ANIMATION) {};   // empty constructor
	ComponentAnimation(JSON_Object* deff, GameObject* parent);
	~ComponentAnimation();

	bool Update(float dt);

	uint getAnimationResource() const { return animation_resource_uuid; }
	void setAnimationResource(uint uuid);

	void Play() { paused = false; }
	void Pause() { paused = true; }
	void Reset() { animTime = 0; }

	void Save(JSON_Object* config);

	bool Finished() const { return false; }
	bool isPaused() const { return paused; }

	float GetAnimTime() const { return animTime; }
	void SetAnimTime(float time) { animTime = time; }

public:

	bool loop = false;
	float speed = 1.0f;

private:

	uint animation_resource_uuid = 0;

	float animTime = 0.0f;
	bool paused = false;
	std::map<uint, uint> bones;

	std::map<uint, AnimSetB> AnimSets;

public:
	// Panel Animation Bound
	bool TestPlay = false;
	bool TestPause = false;
	uint GetCBone(uint index) const 
	{ 
		auto get = bones.find(index);
		if(get != bones.end())
			return get->second;

		return 0;
	}
};

#endif // !_COMPONENT_ANIMATION
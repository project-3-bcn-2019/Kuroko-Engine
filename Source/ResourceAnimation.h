#ifndef __RESOURCEANIMATION_H__
#define __RESOURCEANIMATION_H__

#include "Resource.h"

#include "MathGeoLib/MathGeoLib.h"
#include <map>

struct ComponentKey
{
	double	time;
	std::multimap<int, void*>	ComponentEvents;
		// The first, int is the Event it triggers, depends on every component
		// the second, void* is the value used to perform the "animation", the component should act accordingly and recast it to what you want
};

struct BoneTransform
{
	~BoneTransform();

	bool calcCurrentIndex(float time, bool test);
	void calcTransfrom(float time, bool interpolation);

	void smoothBlending(const float4x4& blendtrans, float time);

	std::string NodeName = "";

	float4x4 lastTransform = float4x4::identity;
	int currentPosIndex = -1;
	int currentRotIndex = -1;
	int currentScaleIndex = -1;
	int nextPosIndex = -1;
	int nextRotIndex = -1;
	int nextScaleIndex = -1;

	int numPosKeys = 0;
	int numScaleKeys = 0;
	int numRotKeys = 0;

	float* PosKeysValues = nullptr;
	double* PosKeysTimes = nullptr;

	float* ScaleKeysValues = nullptr;
	double* ScaleKeysTimes = nullptr;

	float* RotKeysValues = nullptr;
	double* RotKeysTimes = nullptr;
};

class ResourceAnimation : public Resource
{
public:

	ResourceAnimation(resource_deff deff);
	~ResourceAnimation();

	void LoadToMemory();
	void UnloadFromMemory();    
	bool LoadAnimation();
	
	void resetFrames();
	float getDuration() const;

public:

	std::string Parent3dObject;

	float ticks = 0.0f;
	float ticksXsecond = 0.0f;

	int numBones = 0;

	BoneTransform* boneTransformations = nullptr;

	std::multimap <uint, std::vector<ComponentKey>> ComponentAnimations;
		// First is the uuid of the component
		// second is the keyframes of said component

};

#endif // !__RESOURCEANIMATION_H__

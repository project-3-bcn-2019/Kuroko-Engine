#ifndef __COMPONENT_COLLIDER_CUBE__
#define __COMPONENT_COLLIDER_CUBE__
#include "Component.h"
#include "MathGeoLib/MathGeoLib.h"

class GameObject;
class PhysBody;
class PCube;

class ComponentColliderCube : public Component
{
//public:
public:
	ComponentColliderCube();
//	ComponentColliderCube(GameObject* owner);
//	ComponentColliderCube(GameObject* owner,PCube* pcube);
	virtual ~ComponentColliderCube();
//	
//	bool Update(float dt)override;
//
//	void DrawInspectorInfo();
//	void UpdateTransform();
//	bool HasMoved();
//	bool IsBulletStatic();
//public:
//	bool bullet_trans_updated;
//	bool owner_trans_updated;
//	float center_offset[3] = { 0.0f,0.0f,0.0f };
//	float3 dimensions_component = { 1,1,1 };
//	float final_pmatrix[16];
//	float3 cmp_scaling;

};

#endif
#ifndef __COMPONENT_COLLIDER_CUBE__
#define __COMPONENT_COLLIDER_CUBE__
#include "Component.h"
#include "MathGeoLib/MathGeoLib.h"
#include <list>

class GameObject;
class PhysBody;
class PCube;

class ComponentColliderCube : public Component
{
//public:
public:

	ComponentColliderCube(GameObject* parent);
	ComponentColliderCube(JSON_Object* deff, GameObject* parent);

	void OnCollision(GameObject* A, GameObject* B);

	bool Update(float dt);
	void Draw() const;

	virtual ~ComponentColliderCube();
	PhysBody* body;

	std::list<GameObject*> colliding;

	void Save(JSON_Object* config);


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
#ifndef __COMPONENT_COLLIDER_CUBE__
#define __COMPONENT_COLLIDER_CUBE__
#include "Component.h"
#include "MathGeoLib/MathGeoLib.h"
#include <list>

#include "MathGeoLib\Geometry\OBB.h"

class GameObject;
class PhysBody;
class PCube;
class Transform;
enum collision_shape;

class ComponentPhysics : public Component
{
//public:
public:

	ComponentPhysics(GameObject* parent, collision_shape shape, bool is_environment = true);
	ComponentPhysics(JSON_Object* deff, GameObject* parent);

	void OnCollision(GameObject* A, GameObject* B);

	bool Update(float dt);
	void Draw() const;

	virtual ~ComponentPhysics();
	PhysBody* body;

	Transform* transform;
	OBB bounding_box;

	void DrawOBB() const;

	std::list<GameObject*> colliding;

	void Save(JSON_Object* config);

	bool is_environment = true;

	float3 scale;

	void SetSpeed(float x, float y, float z);

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
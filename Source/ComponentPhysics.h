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

	collision_shape shape;

	bool Update(float dt);
	void Draw() const;
	bool DrawInspector(int id = 0) override;

	virtual ~ComponentPhysics();
	PhysBody* body;

	std::list<GameObject*> colliding;

	void Save(JSON_Object* config);

	bool is_environment = true;
	void UpdateTransformsFromPhysics();
	void UpdatePhysicsFromTransforms();

	float3 collider_size = float3(2,2,2);
	float3 offset_scale = float3(1,1,1);
	float3 offset_pos = float3(0,0,0);
	float3 offset_rot = float3(0,0,0);

	void SetSpeed(float x, float y, float z);
	void SetPosition(float x, float y, float z);

	void SetDamping(float new_value, float new_value_angular);
	void SetMass(float new_value);
	float mass = 1;

	void SetStatic(bool is_static);

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
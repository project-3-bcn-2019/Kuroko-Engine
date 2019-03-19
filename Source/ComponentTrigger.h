#ifndef __COMPONENT_TRIGGER__
#define __COMPONENT_TRIGGER__
#include "Component.h"
#include "MathGeoLib/MathGeoLib.h"
#include <list>

#include "MathGeoLib\Geometry\OBB.h"

#include "Bullet/include/BulletCollision/CollisionDispatch/btGhostObject.h"

class GameObject;
class PhysBody;
class PCube;
class Transform;
enum collision_shape;

class ComponentTrigger : public Component
{
	//public:
public:

	ComponentTrigger(GameObject* parent, collision_shape shape);
	ComponentTrigger(JSON_Object* deff, GameObject* parent);

	collision_shape shape;

	bool Update(float dt);
	void Draw() const;
	bool DrawInspector(int id = 0) override;

	virtual ~ComponentTrigger();
	btGhostObject* body;

	std::list<GameObject*> colliding;

	void Save(JSON_Object* config);

	bool is_environment = true;
	void UpdateTransformsFromPhysics();
	void UpdatePhysicsFromTransforms();

	void DeactivateTrigger();
	void ActivateTrigger();
	bool is_trigger_active = true;

	float3 collider_size = float3(2, 2, 2);

	float3 offset_scale = float3(1, 1, 1);
	float3 offset_pos = float3(0, 0, 0);
	float3 offset_rot = float3(0, 0, 0);

	//void SetSpeed(float x, float y, float z);
	//void SetPosition(float x, float y, float z);

	//void SetStatic(bool is_static);

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
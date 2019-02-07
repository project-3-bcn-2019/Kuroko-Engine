#ifndef __PhysBody_H__
#define __PhysBody_H__

#include "Module.h"
#include "GameObject.h"
#include "MathGeoLib/Math/float3.h"

class btRigidBody;
class Module;

// =================================================
struct PhysBody
{
	friend class ModulePhysics3D;
public:
	PhysBody(btRigidBody* body);
	//PhysBody(GameObject* owner, PSphere* primitive);
	//PhysBody(GameObject* owner, PCube* primitive);
	//PhysBody(GameObject* owner, ComponentType type);
	PhysBody();
	~PhysBody();

	void Push(float x, float y, float z);
	void GetTransform(float* matrix) const;
	void SetTransform(float* matrix);
	void SetPos(float x, float y, float z);
	void SetAsSensor(bool is_sensor);
	bool IsSensor() const;
	btRigidBody* GetRigidBody();

	void SetMass(const uint new_mass);
	uint GetMass()const;
	void ActivateGravity(const bool active);
	bool HasGravity()const;
	bool GetRender()const;
	
private:
	btRigidBody * body = nullptr;
	bool is_sensor = false;

	//Unity Elements------------
	//Mass
	uint mass = 0;
	//Affected by gravity?
	bool use_gravity = false;
	bool has_primitive_render = true;
	//Collision Detection
public:
	//Constraints
	bool const_px;
	bool const_py;
	bool const_pz;

	bool const_rx;
	bool const_ry;
	bool const_rz;

	float3 dimensions = { 1,1,1 };
	float3* initial_pos = nullptr;
	bool dead = false;
	GameObject* owner = nullptr;
	//Primitive* primitive_ptr = nullptr;
	//Mesh* mesh_ptr = nullptr;

	//--------------------------	
	std::list<Module*> collision_listeners;
};

#endif // __PhysBody3D_H__
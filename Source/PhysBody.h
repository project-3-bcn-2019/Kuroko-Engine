#ifndef __PhysBody_H__
#define __PhysBody_H__

#include "GameObject.h"


class btRigidBody;
class Module;
class Transform;

// =================================================
struct PhysBody
{
	friend class ModulePhysics3D;
public:
	PhysBody(btRigidBody* body);

	PhysBody();
	~PhysBody();

	void Push(float x, float y, float z);
	void GetTransform(float* matrix) const;
	void SetTransform(float* matrix);
	void SetPos(float x, float y, float z);
	void SetAsSensor(bool is_sensor);
	bool IsSensor() const;

	void SetUser(void* new_user);

	btRigidBody* GetRigidBody();

	void OnCollision(PhysBody* bodyA, PhysBody* bodyB)
	{
		bool a;
		a = 1;
		return;
	};

private:
	btRigidBody * body = nullptr;
	bool is_sensor = true;

	//Unity Elements------------
	//Mass
	uint mass = 0;
	//Affected by gravity?
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
	//Mesh* mesh_ptr = nullptr;

	//--------------------------	
};

#endif // __PhysBody3D_H__
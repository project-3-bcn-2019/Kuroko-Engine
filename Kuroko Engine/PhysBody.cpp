#include "PhysBody.h"
#include "ModulePhysics3D.h"
#include "Application.h"
#include "Component.h"
#include "ComponentTransform.h"
#include "Transform.h"
#include "Bullet/include/btBulletDynamicsCommon.h"

// =================================================
PhysBody::PhysBody(btRigidBody* body) : body(body)
{
	body->setUserPointer(this);
}

PhysBody::PhysBody()
{
	//PSphere* psphere = new PSphere();
	//psphere->radius = 5;
	//App->physics->AddBody(*psphere, 0);
}


// ---------------------------------------------------------
PhysBody::~PhysBody()
{
	delete body;
}

// ---------------------------------------------------------
void PhysBody::Push(float x, float y, float z)
{
	body->applyCentralImpulse(btVector3(x, y, z));
}

// ---------------------------------------------------------
void PhysBody::GetTransform(float* matrix) const
{
	if (body != NULL && matrix != NULL)
	{
		body->getWorldTransform().getOpenGLMatrix(matrix);
	}
}

btRigidBody * PhysBody::GetRigidBody()
{
	return body;
}

//void PhysBody::OnCollision(PhysBody * bodyA, PhysBody * bodyB)
//{
//
//}

// ---------------------------------------------------------
void PhysBody::SetTransform(float* matrix) 
{
	if (body != NULL && matrix != NULL)
	{
		btTransform t;
		t.setFromOpenGLMatrix(matrix);
		body->setWorldTransform(t);
	}
}

// ---------------------------------------------------------
void PhysBody::SetPos(float x, float y, float z)
{
	btTransform t = body->getWorldTransform();
	t.setOrigin(btVector3(x, y, z));
	body->setWorldTransform(t);
}


// ---------------------------------------------------------
void PhysBody::SetAsSensor(bool is_sensor)
{
	if (this->is_sensor != is_sensor)
	{
		this->is_sensor = is_sensor;
		if (is_sensor == true)
			body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
		else
			body->setCollisionFlags(body->getCollisionFlags() &~btCollisionObject::CF_NO_CONTACT_RESPONSE);
	}
}

// ---------------------------------------------------------
bool PhysBody::IsSensor() const
{
	return is_sensor;
}

void PhysBody::SetUser(void * new_user)
{
	body->setUserPointer(new_user);
}







#include "PhysBody.h"
#include "ModulePhysics3D.h"
#include "Application.h"
#include "Component.h"
#include "ComponentTransform.h"
#include "Bullet/include/btBulletDynamicsCommon.h"

// =================================================
PhysBody::PhysBody(btRigidBody* body) : body(body)
{
	body->setUserPointer(this);
}

/*
PhysBody::PhysBody(GameObject * owner, PSphere * primitive)
{
	if (owner->HasChilds()||owner->HasCam()||owner->HasController())
	{
		primitive->has_primitive_render = false;
	}
	if (primitive->isCollider)
	{
		owner->physbody = App->physics->AddBody(*primitive, 0, true);
	}
	else
	{
		owner->physbody = App->physics->AddBody(*primitive, 0, false);
	}
	
	owner->physbody->owner = owner;
}

PhysBody::PhysBody(GameObject * owner, PCube * primitive)
{
	if (owner->HasChilds() || owner->HasCam() || owner->HasController())
	{
		primitive->has_primitive_render = false;
	}
	owner->physbody = App->physics->AddBody(*primitive, 0);
	owner->physbody->owner = owner;

}

PhysBody::PhysBody(GameObject* owner,ComponentType type)
{
	switch (type)
	{
	case COLLIDERSPHERE:
	{
		PSphere* psphere = new PSphere();
		psphere->radius = 5;
		psphere->has_primitive_render = false;
		psphere->mass = 1;
		owner->physbody = App->physics->AddBody(*psphere, 0);
		break;
	}
	case COLLIDERCUBE:
	{
		PCube* pcube = new PCube();
		pcube->dimensions = { 5,5,5 };
		pcube->has_primitive_render = false;
		pcube->mass = 1;
		owner->physbody = App->physics->AddBody(*pcube, 0);		
		break;
	}
	default:
		break;
	}

	owner->physbody->owner = owner;
}
*/
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

void PhysBody::SetMass(const uint new_mass)
{
	mass = new_mass;
}

uint PhysBody::GetMass() const
{
	return mass;
}

void PhysBody::ActivateGravity(const bool active)
{
	use_gravity = active;
	use_gravity ? body->setGravity({ 0,App->physics->GetGravity(),0 }) : body->setGravity({ 0,0,0 });	
}

bool PhysBody::HasGravity()const
{
	return use_gravity;
}

bool PhysBody::GetRender() const
{
	return has_primitive_render;
}



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







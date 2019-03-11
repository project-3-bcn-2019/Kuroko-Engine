#include "Globals.h"
#include "Application.h"
#include "ModulePhysics3D.h"
#include "ModuleInput.h"
#include "ModuleTimeManager.h"
#include "ComponentTransform.h"
#include "ComponentAABB.h"
#include "ComponentPhysics.h"
#include "ComponentTrigger.h"
//#include "ComponentColliderSphere.h"
#include "Component.h"
#include "ModuleCamera3D.h"
#include "Camera.h"
//#include "PanelGame.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "PhysBody.h"

#include "glew-2.1.0\include\GL\glew.h"

#include "Applog.h"

#ifdef _DEBUG
#pragma comment (lib, "Bullet/libx86/BulletDynamics_debug.lib")
#pragma comment (lib, "Bullet/libx86/BulletCollision_debug.lib")
#pragma comment (lib, "Bullet/libx86/LinearMath_debug.lib")
#else
#pragma comment (lib, "Bullet/libx86/BulletDynamics.lib")
#pragma comment (lib, "Bullet/libx86/BulletCollision.lib")
#pragma comment (lib, "Bullet/libx86/LinearMath.lib")
#endif


ModulePhysics3D::ModulePhysics3D(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	name = "physics3d";
}

ModulePhysics3D::~ModulePhysics3D()
{

}

bool ModulePhysics3D::Init()
{
	//CONSOLE_LOG_INFO("Creating 3D Physics simulation");
	bool ret = true;

	return ret;
}

bool ModulePhysics3D::Start()
{
	//CONSOLE_LOG_INFO("Creating Physics environment");
	//World init
	physics_debug = true;

	collision_conf = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collision_conf);
	broad_phase = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
	pdebug_draw = new PDebugDrawer();

	//Creating a world
	world = new btDiscreteDynamicsWorld(dispatcher, broad_phase, solver, collision_conf);

	pdebug_draw->setDebugMode(pdebug_draw->DBG_DrawWireframe);
	world->setDebugDrawer(pdebug_draw);

	world->setGravity(btVector3(0,0,0));
	//Big plane


	return true;
}

update_status ModulePhysics3D::PreUpdate(float dt)
{	
	collisions.clear();


	int numManifolds = world->getDispatcher()->getNumManifolds();
	
	for (int i = 0; i<numManifolds; i++)
	{
		btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* obA = (btCollisionObject*)(contactManifold->getBody0());
		btCollisionObject* obB = (btCollisionObject*)(contactManifold->getBody1());

		int numContacts = contactManifold->getNumContacts();//TO IMPROVE A LOT
		if (numContacts > 0)
		{

			//switch (((Component*)obA->getUserPointer())->getType())
			//{
			//case PHYSICS:
			//	break;
			//case TRIGGER:
			//	break;
			//}

			Component* pbodyA = (Component*)obA->getUserPointer();
			Component* pbodyB = (Component*)obB->getUserPointer();

			Collision cA;
			cA.A = pbodyA->getParent();
			cA.B = pbodyB->getParent();

			Collision cB;
			cB.A = pbodyB->getParent();
			cB.B = pbodyA->getParent();

			collisions.push_back(cA);
			collisions.push_back(cB);
		}
	}
	return UPDATE_CONTINUE;
}

update_status ModulePhysics3D::Update(float dt)
{

	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		physics_debug = !physics_debug;
	//if (App->time->getGameState() == GameState::PLAYING)
	//{
	//	UpdateTransformsFromPhysics();
	//}
	//else
	//{
	//	UpdatePhysicsFromTransforms();
	//}

	world->stepSimulation(dt, 1);


	return UPDATE_CONTINUE;
}

update_status ModulePhysics3D::PostUpdate(float dt)
{
	if (physics_debug)
	{
		world->debugDrawWorld();
	}


	return UPDATE_CONTINUE;
}

void ModulePhysics3D::UpdatePhysics()
{


}

void ModulePhysics3D::CleanUpWorld()
{

	for (int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = world->getCollisionObjectArray()[i];
		world->removeCollisionObject(obj);
	}

	for (std::vector<btTypedConstraint*>::iterator item = constraints.begin(); item != constraints.end(); item++)
	{
		world->removeConstraint((*item));
		delete (*item);
	}

	constraints.clear();

	for (std::vector<btDefaultMotionState*>::iterator item = motions.begin(); item != motions.end(); item++)
		delete (*item);

	motions.clear();

	for (std::vector<btCollisionShape*>::iterator item = shapes.begin(); item != shapes.end(); item++)
		delete (*item);

	shapes.clear();

	for (std::vector<PhysBody*>::iterator item = bodies.begin(); item != bodies.end(); item++)
		delete (*item);

	bodies.clear();
}

bool ModulePhysics3D::CleanUp()
{
	for (int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = world->getCollisionObjectArray()[i];
		world->removeCollisionObject(obj);
	}

	for (std::vector<btTypedConstraint*>::iterator item = constraints.begin(); item != constraints.end(); item++)
	{
		world->removeConstraint((*item));
		delete (*item);
	}

	constraints.clear();

	for (std::vector<btDefaultMotionState*>::iterator item = motions.begin(); item != motions.end(); item++)
		delete (*item);

	motions.clear();

	for (std::vector<btCollisionShape*>::iterator item = shapes.begin(); item != shapes.end(); item++)
		delete (*item);

	shapes.clear();

	for (std::vector<PhysBody*>::iterator item = bodies.begin(); item != bodies.end(); item++)
		delete (*item);

	bodies.clear();

	delete world;

	//delete pdebug_draw;
	delete solver;
	delete broad_phase;
	delete dispatcher;
	delete collision_conf;

	return true;
}


PhysBody * ModulePhysics3D::AddBody(ComponentPhysics* parent, collision_shape shape, bool is_environment)
{
	btCollisionShape* colShape = nullptr;
	switch (shape)
	{
	case COL_CUBE:
		colShape = new btBoxShape(btVector3(1, 1, 1));
		break;
	case COL_CYLINDER:
		colShape = new btCylinderShape(btVector3(1, 1, 1));
		break;
	default:
		colShape = new btBoxShape(btVector3(1, 1, 1));
		break;
	}
	shapes.push_back(colShape);

	btTransform startTransform;
	//startTransform.setFromOpenGLMatrix(((ComponentTransform*)parent->getComponent(TRANSFORM))->global->getMatrix().ptr());

	startTransform.setIdentity();

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	motions.push_back(myMotionState);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(1.0, myMotionState, colShape);//MASS SHOULD BE 0 BUT 1 WORKS SEND HELP
	btRigidBody* body = new btRigidBody(rbInfo);

	body->setFlags(DISABLE_DEACTIVATION);
	body->setActivationState(DISABLE_DEACTIVATION);
	//body->setCollisionFlags(btRigidBody::CollisionFlags::CF_KINEMATIC_OBJECT);
	
	PhysBody* pbody = new PhysBody(body);

	pbody->dimensions = float3(1,1,1);

	world->addRigidBody(body);
	bodies.push_back(pbody);

	return pbody;
}


btGhostObject * ModulePhysics3D::AddTrigger(ComponentTrigger* parent, collision_shape shape)
{
	btCollisionShape* colShape = nullptr;
	switch (shape)
	{
	case COL_CUBE:
		colShape = new btBoxShape(btVector3(1, 1, 1));
		break;
	case COL_CYLINDER:
		colShape = new btCylinderShape(btVector3(1, 1, 1));
		break;
	default:
		colShape = new btBoxShape(btVector3(1, 1, 1));
		break;
	}
	shapes.push_back(colShape);

	btTransform startTransform;

	startTransform.setIdentity();

	btGhostObject* body = new btGhostObject();
	body->setCollisionShape(colShape);
	body->setWorldTransform(startTransform);
	
	body->setActivationState(DISABLE_DEACTIVATION);
	body->setCollisionFlags(btRigidBody::CollisionFlags::CF_NO_CONTACT_RESPONSE);

	world->addCollisionObject(body);
	triggers.push_back(body);

	return body;
}

void ModulePhysics3D::DeleteBody(PhysBody * body_to_delete)
{
	world->removeCollisionObject(body_to_delete->body);

	delete body_to_delete->body->getCollisionShape();
	shapes.erase(std::remove(begin(shapes), end(shapes), body_to_delete->body->getCollisionShape()), end(shapes));
	delete body_to_delete->body->getMotionState();
	motions.erase(std::remove(begin(motions), end(motions), body_to_delete->body->getMotionState()), end(motions));
	delete body_to_delete;
	bodies.erase(std::remove(begin(bodies), end(bodies), body_to_delete), end(bodies));

}

void ModulePhysics3D::DeleteTrigger(ComponentTrigger * component)
{
	world->removeCollisionObject(component->body);
	
	delete component->body->getCollisionShape();
	shapes.erase(std::remove(begin(shapes), end(shapes), component->body->getCollisionShape()), end(shapes));
	delete component->body;
	triggers.erase(std::remove(begin(triggers), end(triggers), component->body), end(triggers));
}


void ModulePhysics3D::GetCollisionsFromObject(std::list<Collision>& list_to_fill, GameObject * to_get)
{

	for (std::list<Collision>::iterator item = collisions.begin(); item != collisions.end(); ++item)
	{
		if ((*item).A == to_get)
		{
			list_to_fill.push_back(*item);
		}
	}


}




//=============================================================================================================

void PDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{

	glLineWidth(2.0f);

	glBegin(GL_LINES);

	glColor4f(color.x(),color.y(),color.z(),255);

	glVertex3f(from.x(), from.y(), from.z());
	glVertex3f(to.x(),to.y(),to.z());

	glEnd();

	//debug_line.origin.Set(from.getX(), from.getY(), from.getZ());
	//debug_line.destination.Set(to.getX(), to.getY(), to.getZ());
	//debug_line.color.Set(color.getX(), color.getY(), color.getZ());
	//debug_line.Render();
	return;

}

void PDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
	//point.SetPos(PointOnB.getX(), PointOnB.getY(), PointOnB.getZ());
	//point.color.Set(color.getX(), color.getY(), color.getZ());
	//point.Render();
}

void PDebugDrawer::reportErrorWarning(const char* warningString)
{
	//LOG("Bullet warning: %s", warningString);
}

void PDebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
	//LOG("Bullet draw text: %s", textString);
}

void PDebugDrawer::setDebugMode(int debugMode)
{
	mode = (DebugDrawModes)debugMode;
}

int	 PDebugDrawer::getDebugMode() const
{
	return mode;
}
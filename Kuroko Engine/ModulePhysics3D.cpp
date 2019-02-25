#include "Globals.h"
#include "Application.h"
#include "ModulePhysics3D.h"
#include "ModuleInput.h"
#include "ModuleTimeManager.h"
#include "ComponentTransform.h"
#include "ComponentColliderCube.h"
//#include "ComponentColliderSphere.h"
#include "Component.h"
#include "ModuleCamera3D.h"
#include "Camera.h"
//#include "PanelGame.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "PhysBody.h"

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
	//Big plane


	return true;
}

update_status ModulePhysics3D::PreUpdate(float dt)
{
	int numManifolds = world->getDispatcher()->getNumManifolds();
	for (int i = 0; i<numManifolds; i++)
	{
		btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* obA = (btCollisionObject*)(contactManifold->getBody0());
		btCollisionObject* obB = (btCollisionObject*)(contactManifold->getBody1());

		int numContacts = contactManifold->getNumContacts();
		if (numContacts > 0)
		{
			PhysBody* pbodyA = (PhysBody*)obA->getUserPointer();
			PhysBody* pbodyB = (PhysBody*)obB->getUserPointer();

			if (pbodyA && pbodyB)
			{
				for (std::vector<Module*>::iterator item = pbodyA->collision_listeners.begin(); item != pbodyA->collision_listeners.begin(); item++)
				{
					(*item)->OnCollision(pbodyA, pbodyB);
				}

				for (std::vector<Module*>::iterator item = pbodyB->collision_listeners.begin(); item != pbodyB->collision_listeners.begin(); item++)
				{
					(*item)->OnCollision(pbodyB, pbodyA);
				}
			}
		}
	}
	return UPDATE_CONTINUE;
}

update_status ModulePhysics3D::Update(float dt)
{
	return UPDATE_CONTINUE;
}

update_status ModulePhysics3D::PostUpdate(float dt)
{
	return UPDATE_CONTINUE;
}

void ModulePhysics3D::UpdatePhysics()
{
}

bool ModulePhysics3D::CleanUp()
{
	return false;
}

void ModulePhysics3D::CleanUpWorld()
{
}

PhysBody * ModulePhysics3D::AddBody()
{
	return nullptr;
}

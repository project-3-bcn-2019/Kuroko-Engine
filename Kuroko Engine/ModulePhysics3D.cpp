#include "Globals.h"
#include "Application.h"
#include "ModulePhysics3D.h"
#include "ModuleInput.h"
#include "ModuleTimeManager.h"
#include "ComponentTransform.h"
#include "ComponentAABB.h"
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
	world->stepSimulation(App->dt, 1);

	int numManifolds = world->getDispatcher()->getNumManifolds();
	
	for (int i = 0; i<numManifolds; i++)
	{
		btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* obA = (btCollisionObject*)(contactManifold->getBody0());
		btCollisionObject* obB = (btCollisionObject*)(contactManifold->getBody1());

		int numContacts = contactManifold->getNumContacts();//TO IMPROVE A LOT
		if (numContacts > 0)
		{
			ComponentColliderCube* pbodyA = (ComponentColliderCube*)obA->getUserPointer();
			ComponentColliderCube* pbodyB = (ComponentColliderCube*)obB->getUserPointer();

			if (pbodyA && pbodyB)
			{
				pbodyA->OnCollision(pbodyA->getParent(), pbodyB->getParent());
				pbodyB->OnCollision(pbodyB->getParent(), pbodyA->getParent());
			}
		}
	}
	return UPDATE_CONTINUE;
}

update_status ModulePhysics3D::Update(float dt)
{
	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		physics_debug = !physics_debug;




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
	//float *matrix = new float[16];
	for (std::vector<PhysBody*>::iterator item = bodies.begin(); item != bodies.end(); item++)
	{
		float* matrix = new float[16];
		//(*item)->GetTransform(matrix);

		GameObject* obj = ((ComponentColliderCube*)(*item)->body->getUserPointer())->getParent();

		ComponentTransform* transform = (ComponentTransform*)obj->getComponent(TRANSFORM);

		matrix = transform->global->getMatrix().ptr();

		btTransform* t = new btTransform();
	
		t->setFromOpenGLMatrix(matrix);

		t->setRotation(btQuaternion(btVector3(0,1,0),90));
		
		(*item)->body->setWorldTransform(*t);
		
		delete t;

		////if ((*item)->owner != nullptr)
		//{
		//	//Get matrix from bullet physics
		//	float4x4 final_matrix4x4;
		//	float* matrix = new float[16];
		//	(*item)->GetTransform(matrix);
		//	//(*item)->owner->comp_transform->GetGlobalMatrix();
		//	//Set Rotations
		//	final_matrix4x4[0][0] = matrix[0];	final_matrix4x4[0][1] = matrix[1];	final_matrix4x4[0][2] = matrix[2];
		//	final_matrix4x4[1][0] = matrix[4];	final_matrix4x4[1][1] = matrix[5];	final_matrix4x4[1][2] = matrix[6];
		//	final_matrix4x4[2][0] = matrix[8];	final_matrix4x4[2][1] = matrix[9];	final_matrix4x4[2][2] = matrix[10];
		//	final_matrix4x4.Transpose();
		//	float3 pos = { matrix[12], matrix[13], matrix[14] }; //float3(0, 0, 0);// (*item)->owner->comp_transform->GetLocalMatrix().Col3(3);
		//	static float3 init_local_pos;
		//	if ((*item)->initial_pos == nullptr)
		//	{
		//		(*item)->initial_pos = new float3(matrix[12], matrix[13], matrix[14]);
		//		init_local_pos = pos;
		//	}
		//	else
		//	{
		//		float3 local_pos = { matrix[12], matrix[13], matrix[14] };
		//		pos = init_local_pos + local_pos - *(*item)->initial_pos;
		//	}
		//	//Matrix Translation and size
		//	float* final_pos = new float[3];
		//	final_pos[0] = pos.x;
		//	final_pos[1] = pos.y;
		//	final_pos[2] = pos.z;
		//	//}
		//	//Transform + size
		//	final_matrix4x4[0][3] = final_pos[0];
		//	final_matrix4x4[1][3] = final_pos[1];
		//	final_matrix4x4[2][3] = final_pos[2];
		//	final_matrix4x4[3][0] = 1;				
		//	final_matrix4x4[3][1] = 1;	
		//	final_matrix4x4[3][2] = 1;	
		//	final_matrix4x4[3][3] = matrix[15];
		//	(*item)->SetPos(final_pos[0], final_pos[1], final_pos[2]);
		//}
	}
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

PhysBody * ModulePhysics3D::AddBody(GameObject* parent)
{
	ComponentAABB* box = (ComponentAABB*)parent->getComponent(C_AABB);
	if (box == nullptr)
	{
		box = (ComponentAABB*)parent->addComponent(C_AABB);
		box->getAABB()->maxPoint = float3(0.5, 0.5, 0.5);
		box->getAABB()->minPoint = float3(-0.5, -0.5, -0.5);
	}
	
	btCollisionShape* colShape = new btBoxShape(btVector3(box->getAABB()->Size().x*0.5, box->getAABB()->Size().y*0.5, box->getAABB()->Size().z*0.5));
	shapes.push_back(colShape);

	btTransform startTransform;
	//startTransform.setFromOpenGLMatrix(((ComponentTransform*)parent->getComponent(TRANSFORM))->global->getMatrix().ptr());

	startTransform.setIdentity();

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	motions.push_back(myMotionState);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(1.0, myMotionState, colShape);//MASS SHOULD BE 0 BUT 1 WORKS SEND HELP

	btRigidBody* body = new btRigidBody(rbInfo);
	PhysBody* pbody = new PhysBody(body);

	pbody->dimensions = box->getAABB()->Size();

	world->addRigidBody(body);
	bodies.push_back(pbody);

	return pbody;
}

void ModulePhysics3D::DeleteBody(PhysBody * body_to_delete)
{
}




//=============================================================================================================

void PDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	debug_line.origin.Set(from.getX(), from.getY(), from.getZ());
	debug_line.destination.Set(to.getX(), to.getY(), to.getZ());
	debug_line.color.Set(color.getX(), color.getY(), color.getZ());
	debug_line.Render();
}

void PDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
	point.SetPos(PointOnB.getX(), PointOnB.getY(), PointOnB.getZ());
	point.color.Set(color.getX(), color.getY(), color.getZ());
	point.Render();
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
#ifndef __MODULE_PHYSICS_3D__
#define __MODULE_PHYSICS_3D__

#include "Module.h"
#include "Globals.h"
#include "MathGeoLib/MathGeoLib.h"



#include "Bullet/include/btBulletDynamicsCommon.h"

// Recommended scale is 1.0f == 1 meter, no less than 0.2 objects
#define GRAVITY btVector3(0.0f, -10.0f, 0.0f) 

class GameObject;
struct PhysBody;

struct Collision
{
	GameObject* A;
	GameObject* B;
};

class PDebugDrawer : public btIDebugDraw
{
public:
	PDebugDrawer()
	{}

	void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
	void reportErrorWarning(const char* warningString);
	void draw3dText(const btVector3& location, const char* textString);
	void setDebugMode(int debugMode);
	int	 getDebugMode() const;

	DebugDrawModes mode;
};
class ModulePhysics3D : public Module
{
public:	
	ModulePhysics3D(Application* app, bool start_enabled = true);
	~ModulePhysics3D();

	bool Init();
	bool Start();
	//void DrawModuleConfig()override;

	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);

	void UpdatePhysics();

	bool CleanUp();
	void CleanUpWorld();

	PhysBody* AddBody(GameObject* parent);
	void DeleteBody(PhysBody* body_to_delete);

	std::list<Collision> collisions;
	void GetCollisionsFromObject(std::list<Collision> &list_to_fill,GameObject* to_get);

	
private:

	bool physics_debug = true;
	//physics
	btDefaultCollisionConfiguration*	collision_conf;
	btCollisionDispatcher*				dispatcher;
	btBroadphaseInterface*				broad_phase;
	btSequentialImpulseConstraintSolver*solver;
	PDebugDrawer*						pdebug_draw;
	btDiscreteDynamicsWorld*			world;
	

	std::vector<btCollisionShape*>		shapes;
	std::vector<PhysBody*>				bodies;
	std::vector<btDefaultMotionState*>	motions;
	std::vector<btTypedConstraint*>		constraints;

};




#endif


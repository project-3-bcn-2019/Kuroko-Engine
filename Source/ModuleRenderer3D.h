#ifndef __ModuleRenderer_H__
#define __ModuleRenderer_H__

#include "Module.h"
#include "Globals.h"
#include "Light.h"

#include "Math.h"

#include <list>
#include <queue>
#include "Particle.h"
#include "GameObject.h"

#define MAX_LIGHTS 8

struct ParticlePriority
{
	bool operator()(const Particle* particle1, const Particle* particle2)const
	{
		return  particle1->DistanceToCamera() < particle2->DistanceToCamera();
	}
};

struct MeshPriority
{
	bool operator()(const Component* comp1, const Component* comp2)const
	{
		return  comp1->getParent()->distanceToCamera() <  comp2->getParent()->distanceToCamera();
	}
};

struct UIPriority
{
	bool operator()(const Component* UI1, const Component* UI2)const
	{
		return  UI1->getDepth() < UI1->getDepth();
	}
};



class Camera;
class Particle;
class ComponentMesh;

class ModuleRenderer3D : public Module
{
public:
	ModuleRenderer3D(Application* app, bool start_enabled = true);
	~ModuleRenderer3D() {};

	bool Init(const JSON_Object* config);
	update_status PreUpdate(float dt);
	update_status PostUpdate(float dt);
	bool CleanUp();
	void Render();

	void OnResize(int width, int height);

	SDL_GLContext getContext() const	{ return context; }

	void DirectDrawCube(float3& size, float3& pos) const;
	void DrawDirectAABB(AABB aabb) const;

public:

	SDL_GLContext context;

	std::priority_queue<Particle*, std::vector<Particle*>, ParticlePriority> orderedParticles;
	std::priority_queue<Component*, std::vector<Component*>, UIPriority> orderedUI;
	std::vector<Component*> opaqueMeshes;
	std::priority_queue<Component*, std::vector<Component*>, MeshPriority> translucentMeshes;
	std::vector<ComponentMesh*> selected_meshes_to_draw;


private:

	Light lights[MAX_LIGHTS];
};

#endif
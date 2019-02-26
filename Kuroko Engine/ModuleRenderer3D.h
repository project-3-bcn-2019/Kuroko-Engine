#ifndef __ModuleRenderer_H__
#define __ModuleRenderer_H__

#include "Module.h"
#include "Globals.h"
#include "Light.h"

#include "Math.h"

#include <list>

#define MAX_LIGHTS 8

class ModuleRenderer3D : public Module
{
public:
	ModuleRenderer3D(Application* app, bool start_enabled = true);
	~ModuleRenderer3D() {};

	bool Init(const JSON_Object* config);
	bool Start();
	update_status PreUpdate(float dt);
	update_status PostUpdate(float dt);
	bool CleanUp();

	void OnResize(int width, int height);

	SDL_GLContext getContext() const	{ return context; }

	void DirectDrawCube(float3& size, float3& pos) const;
	void DrawDirectAABB(AABB aabb) const;

private:

	Light lights[MAX_LIGHTS];
	SDL_GLContext context;
};

#endif
#ifndef _MODULE_CAMERA
#define _MODULE_CAMERA
#include "Module.h"
#include "Globals.h"

#include "MathGeoLib\Math\float4x4.h"

#include <list>
#include <array>

class Camera;

#define MIN_H_FOV 45.0f
#define MAX_H_FOV 154.0f
#define MIN_V_FOV 50.0f
#define MAX_V_FOV 103.0f

class ModuleCamera3D : public Module
{
public:
	ModuleCamera3D(Application* app, bool start_enabled = true);
	~ModuleCamera3D();

	bool Init(const JSON_Object* config);
	bool Start();
	update_status Update(float dt);
	bool CleanUp();

public:
	Camera* editor_camera				= nullptr;
	Camera* game_camera					= nullptr;

	Camera* override_editor_cam_culling = nullptr;
	Camera* current_camera				= nullptr;

	std::list<Camera*> game_cameras;
	std::array<Camera*, 6> viewports;

	float editor_cam_speed = 2.5f;
	float editor_cam_rot_speed = 0.25f;
};

#endif
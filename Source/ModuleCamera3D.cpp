#include "Globals.h"
#include "Application.h"
#include "ModuleCamera3D.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "Applog.h"
#include "ComponentTransform.h"
#include "Camera.h"
#include "GameObject.h"
#include "ComponentCamera.h"
#include "Transform.h"
#include "ModuleWindow.h"
#include "Material.h"
#include "ModuleUI.h"

#include "glew-2.1.0\include\GL\glew.h"




ModuleCamera3D::ModuleCamera3D(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	name = "camera";
}

ModuleCamera3D::~ModuleCamera3D()
{}

// -----------------------------------------------------------------
bool ModuleCamera3D::Init(const JSON_Object* config)
{
	app_log->AddLog("Setting up the camera");
	game_camera = editor_camera = new Camera(float3(-2.0, 67.0f, -70.0f), float3::zero);
	editor_camera->active = true;

	viewports[VP_RIGHT] = new Camera(float3(10.0, 0.0f, 0.0f ), float3::zero);
	viewports[VP_LEFT]	= new Camera(float3(-10.0, 0.0f, 0.0f), float3::zero);
	viewports[VP_UP]	= new Camera(float3(0.0, 10.0f, 0.0f ), float3::zero);
	viewports[VP_DOWN]	= new Camera(float3(0.0, -10.0f, 0.0f), float3::zero);
	viewports[VP_FRONT] = new Camera(float3(0.0, 0.0f, 10.0f ), float3::zero);
	viewports[VP_BACK]	= new Camera(float3(0.0, 0.0f, -10.0f), float3::zero);

	return true;
}

bool ModuleCamera3D::Start()
{
	for (int i = 0; i < 6; i++)
		viewports[i]->initFrameBuffer();

	if(!App->is_game)
		editor_camera->initFrameBuffer();

	return true;
}

// -----------------------------------------------------------------
bool ModuleCamera3D::CleanUp()
{
	for (auto it = game_cameras.begin(); it != game_cameras.end(); it++)
		delete *it;

	game_cameras.clear();
	app_log->AddLog("Cleaning camera");
	return true;
}

// -----------------------------------------------------------------
update_status ModuleCamera3D::Update(float dt)
{

	if (editor_camera->active)
	{
		// Movement
		float3 displacement = float3::zero;
		float speed = editor_cam_speed * dt;
		bool orbit = App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT;

		if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT)
			speed = 5.0f * speed;

		if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT)
		{
			if (App->input->GetKey(SDL_SCANCODE_E) == KEY_REPEAT) { displacement.y += speed; };
			if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT) { displacement.y -= speed; };

			if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) { displacement += editor_camera->Z * speed; };
			if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) { displacement -= editor_camera->Z * speed; };

			if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) { displacement += editor_camera->X * speed; };
			if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) { displacement -= editor_camera->X * speed; };
		}

		// panning
		if (App->input->GetMouseButton(SDL_BUTTON_MIDDLE) == KEY_REPEAT && !orbit)
		{
			int dx = App->input->GetMouseXMotion();
			int dy = App->input->GetMouseYMotion();

			if (dx)		displacement += editor_camera->X * dx * speed;
			if (dy)		displacement += editor_camera->Y * dy * speed;
		}

		editor_camera->Move(displacement);

		// Rotation / Orbit

		if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT || (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_REPEAT && orbit))
		{
			int dx = 0; int dy = 0;
			dx = -App->input->GetMouseXMotion();
			dy = App->input->GetMouseYMotion();

			float module = module = (editor_camera->Reference - editor_camera->getFrustum()->pos).Length();
			float3 X = editor_camera->X; float3 Y = editor_camera->Y; float3 Z = editor_camera->Z;

			if (orbit)
				editor_camera->LookAtSelectedGeometry();

			if (dx)
			{
				X = Quat::RotateY(dx * editor_cam_rot_speed * DEGTORAD) * X;
				Y = Quat::RotateY(dx * editor_cam_rot_speed * DEGTORAD) * Y;
				Z = Quat::RotateY(dx * editor_cam_rot_speed * DEGTORAD) * Z;
			}

			if (dy)
			{
				Y = Quat::RotateAxisAngle(X, dy * editor_cam_rot_speed * DEGTORAD) * Y;
				Z = Quat::RotateAxisAngle(X, dy * editor_cam_rot_speed * DEGTORAD) * Z;

				if (Y.y < 0.0f)
				{
					Z = float3(0.0f, Z.y > 0.0f ? 1.0f : -1.0f, 0.0f);
					Y = Z.Cross(X);
				}
			}

			if (!IsNan(-X.x) && !IsNan(-Y.x) && !IsNan(-Z.x))
			{
				editor_camera->X = X; editor_camera->Y = Y; editor_camera->Z = Z;
			}

			if (!orbit)
				editor_camera->Reference = editor_camera->getFrustum()->pos + editor_camera->Z * module;
			else
				editor_camera->getFrustum()->pos = editor_camera->Reference - editor_camera->Z * module;
		}

		// Zooming

		if (int mouse_z = App->input->GetMouseZ())
		{
			if (mouse_z > 0)
			{
				if ((editor_camera->Reference - editor_camera->getFrustum()->pos).Length() > 1.0f)
					editor_camera->getFrustum()->pos += editor_camera->Z * (0.3f + ((editor_camera->Reference - editor_camera->getFrustum()->pos).Length() / 20));
			}
			else
				editor_camera->getFrustum()->pos -= editor_camera->Z * (0.3f + ((editor_camera->Reference - editor_camera->getFrustum()->pos).Length() / 20));
		}

		// Focus 
		if (App->input->GetKey(SDL_SCANCODE_F) == KEY_DOWN && !editor_camera->getParent())
			editor_camera->FitToSizeSelectedGeometry();

		// Recalculate matrix -------------
		editor_camera->updateFrustum();


	}

	return UPDATE_CONTINUE;	
}




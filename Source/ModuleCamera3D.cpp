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
	background_camera = selected_camera = editor_camera = new Camera(float3(-2.0, 2.0f, -5.0f), float3::zero);
	editor_camera->active = true;

	viewports[VP_RIGHT] = new Camera(float3(10.0, 0.0f, 0.0f ), float3::zero);
	viewports[VP_LEFT]	= new Camera(float3(-10.0, 0.0f, 0.0f), float3::zero);
	viewports[VP_UP]	= new Camera(float3(0.0, 10.0f, 0.0f ), float3::zero);
	viewports[VP_DOWN]	= new Camera(float3(0.0, -10.0f, 0.0f), float3::zero);
	viewports[VP_FRONT] = new Camera(float3(0.0, 0.0f, 10.0f ), float3::zero);
	viewports[VP_BACK]	= new Camera(float3(0.0, 0.0f, -10.0f), float3::zero);

	camera_speed = 2.5f;
	camera_rotation_speed = 0.25f;

	return true;
}

bool ModuleCamera3D::Start()
{
	for (int i = 0; i < 6; i++)
		viewports[i]->initFrameBuffer();

	editor_camera->frustum->pos = float3(1, 50, -100);
	editor_camera->LookAt(float3(0, 0, 0));


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

void ModuleCamera3D::updateFOVfromWindow()
{
	float aspect_ratio = (float)App->window->main_window->width / (float)App->window->main_window->height;
	background_camera->getFrustum()->horizontalFov = 2* math::Atan(math::Tan(background_camera->getFrustum()->verticalFov/2) * aspect_ratio);
}

// -----------------------------------------------------------------
update_status ModuleCamera3D::Update(float dt)
{
	updateFOVfromWindow();

	// Not allow camera to be modified if UI is being operated
	if (selected_camera && (!ImGui::IsMouseHoveringAnyWindow() || (ImGui::IsMouseHoveringAnyWindow() && selected_camera->draw_in_UI)) && !App->gui->disable_keyboard_control)
	{
		// Movement
		float3 displacement = float3::zero;
		float speed = camera_speed * dt;
		bool orbit = (App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT && selected_camera == editor_camera);

		ComponentTransform* transform = nullptr;
		if (selected_camera->getParent())
			transform = (ComponentTransform*)selected_camera->getParent()->getParent()->getComponent(TRANSFORM);

		if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT)
			speed = 5.0f * speed;

		if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT)
		{
			if (App->input->GetKey(SDL_SCANCODE_E) == KEY_REPEAT) { displacement.y += speed; };
			if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT) { displacement.y -= speed; };

			if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) { displacement += selected_camera->Z * speed; };
			if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) { displacement -= selected_camera->Z * speed; };

			if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) { displacement += selected_camera->X * speed; };
			if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) { displacement -= selected_camera->X * speed; };
		}

		// panning
		if (App->input->GetMouseButton(SDL_BUTTON_MIDDLE) == KEY_REPEAT && !orbit)
		{
			int dx = App->input->GetMouseXMotion();
			int dy = App->input->GetMouseYMotion();

			if (dx)		displacement += selected_camera->X * dx * speed;
			if (dy)		displacement += selected_camera->Y * dy * speed;
		}

		if (transform)
		{
			if (transform->getParent()->is_static)
				displacement.x = displacement.y = displacement.z = 0;
			else
			{
				if (transform->constraints[0][0]) displacement.x = 0;
				if (transform->constraints[0][1]) displacement.y = 0;
				if (transform->constraints[0][2]) displacement.z = 0;
			}
		}

		selected_camera->Move(displacement);

		// Rotation / Orbit

		if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT || (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_REPEAT && orbit))
		{
			int dx = 0; int dy = 0;

			if (transform)
			{
				if (!transform->getParent()->is_static)
				{
					dx = transform->constraints[1][1] ? 0 : -App->input->GetMouseXMotion();
					dy = transform->constraints[1][0] ? 0 : App->input->GetMouseYMotion();
				}
			}
			else
			{
				if (!selected_camera->IsViewport())
				{
					dx = -App->input->GetMouseXMotion();
					dy = App->input->GetMouseYMotion();
				}
			}

			float module = module = (selected_camera->Reference - selected_camera->getFrustum()->pos).Length();
			float3 X = selected_camera->X; float3 Y = selected_camera->Y; float3 Z = selected_camera->Z;

			if(orbit)
				selected_camera->LookAtSelectedGeometry();

			if (dx)
			{
				X = Quat::RotateY(dx * camera_rotation_speed * DEGTORAD) * X;
				Y = Quat::RotateY(dx * camera_rotation_speed * DEGTORAD) * Y;
				Z = Quat::RotateY(dx * camera_rotation_speed * DEGTORAD) * Z;
			}

			if (dy)
			{
				Y = Quat::RotateAxisAngle(X, dy * camera_rotation_speed * DEGTORAD) * Y;
				Z = Quat::RotateAxisAngle(X, dy * camera_rotation_speed * DEGTORAD) * Z;

				if (Y.y < 0.0f)
				{
					Z = float3(0.0f, Z.y > 0.0f ? 1.0f : -1.0f, 0.0f);
					Y = Z.Cross(X);
				}
			}

			if (!IsNan(-X.x) && !IsNan(-Y.x) && !IsNan(-Z.x))
			{
				selected_camera->X = X; selected_camera->Y = Y; selected_camera->Z = Z;
			}

			if (!orbit)
				selected_camera->Reference = selected_camera->getFrustum()->pos + selected_camera->Z * module;
			else
				selected_camera->getFrustum()->pos = selected_camera->Reference - selected_camera->Z * module;
		}

		// Zooming

		if (int mouse_z = App->input->GetMouseZ())
		{
			if (mouse_z > 0)
			{
				if ((selected_camera->Reference - selected_camera->getFrustum()->pos).Length() > 1.0f)
					selected_camera->getFrustum()->pos += selected_camera->Z * (0.3f + ((selected_camera->Reference - selected_camera->getFrustum()->pos).Length() / 20));
			}
			else
				selected_camera->getFrustum()->pos -= selected_camera->Z * (0.3f + ((selected_camera->Reference - selected_camera->getFrustum()->pos).Length() / 20));
		}

		// Focus 
		if (App->input->GetKey(SDL_SCANCODE_F) == KEY_DOWN && !selected_camera->getParent())
			selected_camera->FitToSizeSelectedGeometry();

		// Recalculate matrix -------------
		selected_camera->updateFrustum();

		if (transform)
		{
			transform->local->setPosition(selected_camera->getFrustum()->pos);
			transform->local->setRotation(Quat::LookAt(transform->local->Forward(), selected_camera->getFrustum()->front, transform->local->Up(), float3::unitY) * transform->local->getRotation());
			transform->local->CalculateMatrix();
			transform->LocalToGlobal();
		}

	}

	return UPDATE_CONTINUE;	
}




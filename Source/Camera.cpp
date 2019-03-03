#include "Camera.h"
#include "GameObject.h"
#include "Application.h"
#include "ComponentTransform.h"
#include "Transform.h"
#include "ModuleScene.h"
#include "Material.h"
#include "ModuleWindow.h"
#include "ModuleCamera3D.h"
#include "ComponentAABB.h"


#include "MathGeoLib\Math\Quat.h"
#include "MathGeoLib\Geometry\Plane.h"

#include "glew-2.1.0\include\GL\glew.h"

Camera::Camera(float3 position, float3 reference, math::FrustumType f_type)
{
	frustum = new Frustum();
	frustum->pos = position;
	frustum->front = float3::unitZ;
	frustum->up = float3::unitY;
	frustum->type = f_type;

	if (f_type == math::FrustumType::OrthographicFrustum)
	{
		frustum->orthographicHeight = INIT_ORT_SIZE;			frustum->orthographicWidth = INIT_ORT_SIZE;
		frustum->nearPlaneDistance = INIT_N_PLANE;				frustum->farPlaneDistance = INIT_F_PLANE / 2;
	}
	else
	{
		frustum->verticalFov = DEGTORAD * INIT_VER_FOV;			frustum->horizontalFov = DEGTORAD * INIT_HOR_FOV;
		frustum->nearPlaneDistance = INIT_N_PLANE;				frustum->farPlaneDistance = INIT_F_PLANE;
	}

	LookAt(reference);
	updateFrustum();

	App->camera->game_cameras.push_back(this);
}

Camera::~Camera()
{
	if (frustum) delete frustum;
	if (frame_buffer) delete frame_buffer;
}

FrameBuffer::~FrameBuffer()
{
	if (tex) delete tex;
	if (depth_tex) delete depth_tex;
}


// -----------------------------------------------------------------
void Camera::LookAt(const float3 &Spot)
{
	Reference = Spot;

	Z = (Reference - frustum->pos).Normalized();
	X = float3::unitY.Cross(Z).Normalized();
	Y = Z.Cross(X);

	updateFrustum();

}


void Camera::Move(const float3 &Movement)
{
	frustum->pos += Movement;
	Reference += Movement;

	updateFrustum();
}

void Camera::LookAtSelectedGeometry()
{
	if (!App->scene->selected_obj.empty()) {
		GameObject* selected_obj = *App->scene->selected_obj.begin();
		LookAt(selected_obj->getCentroid());
	}

	updateFrustum();
}

void Camera::FitToSizeSelectedGeometry(float distance)
{
	if (GameObject* selected_obj = *App->scene->selected_obj.begin())
	{
		AABB* aabb = ((ComponentAABB*)selected_obj->getComponent(C_AABB))->getAABB(); 
		float3 new_pos = aabb->Centroid() + aabb->HalfSize() + float3(distance, distance, distance);
		new_pos = Quat::RotateY(((ComponentTransform*)selected_obj->getComponent(TRANSFORM))->global->getRotationEuler().y) * new_pos;
		frustum->pos = new_pos;
		LookAt(selected_obj->getCentroid());
	}
	else
		LookAt(float3::zero);

	updateFrustum();
}

void Camera::Reset()
{
	frustum->pos = float3(1.0f, 1.0f, 5.0f);
	Reference = float3(0.0f, 0.0f, 0.0f);

	Z = (frustum->pos - Reference).Normalized();
	X = float3::unitY.Cross(Z).Normalized();
	Y = Z.Cross(X);

	LookAt(Reference);

	updateFrustum();
}

bool Camera::IsViewport()
{
	for (int i = 0; i < 6; i++)
		if (App->camera->viewports[i] == this)
			return true;

	return false;
}

ViewportDir Camera::getViewportDir()
{
	for (int i = 0; i < 6; i++)
		if (App->camera->viewports[i] == this)
			return (ViewportDir)i;

	return VP_NONE;
}


std::string Camera::getViewportDirString()
{
	std::string ret;
	switch (getViewportDir())
	{
	case VP_RIGHT:	ret = "Right"; break;
	case VP_LEFT:	ret = "Left"; break;
	case VP_UP:		ret = "Up"; break;
	case VP_DOWN:	ret = "Down"; break;
	case VP_FRONT:	ret = "Front"; break;
	case VP_BACK:	ret = "Back"; break;
	default: break;
	}
	return ret;
}

bool Camera::frustumCull(const OBB& obb)
{
	if (this == App->camera->background_camera && App->camera->override_editor_cam_culling)
		return App->camera->override_editor_cam_culling->frustumCull(obb);

	for (int i = 0; i < 6; i++)
	{
		Plane plane = frustum->GetPlane(i);
		uint count = 0;
		for (int j = 0; j < 8; j++)
		{
			if (plane.IsOnPositiveSide(obb.CornerPoint(j)))
				count++;
			else break;
		}

		if (count == 8)
			return false;
	}

	return true;
}

void Camera::updateFrustum()
{
	frustum->front = Z;
	frustum->up = Y;
}

float * Camera::GetProjectionMatrix()const
{
	static float4x4 matrix;
	matrix = getFrustum()->ProjectionMatrix();
	matrix.Transpose();
	return (float*)matrix.v;
}

float * Camera::GetViewMatrix()const
{
	static float4x4 matrix;
	matrix = getFrustum()->ViewMatrix();
	matrix.Transpose();
	return  (float*)matrix.v;
}

void  Camera::setFov(float hor_fov, float ver_fov)
{
	frustum->horizontalFov = hor_fov; frustum->verticalFov = ver_fov;
}

void  Camera::setPlaneDistance(float n_plane, float f_plane)
{
	frustum->nearPlaneDistance = n_plane; frustum->farPlaneDistance = f_plane;
}


void Camera::initFrameBuffer()
{
	frame_buffer = new FrameBuffer();
	frame_buffer->size_x = App->window->main_window->width; frame_buffer->size_y = App->window->main_window->height;

	//// set frame buffer texture
	frame_buffer->tex = new Texture();
	glGenTextures(1, &frame_buffer->tex->gl_id);
	glBindTexture(GL_TEXTURE_2D, frame_buffer->tex->gl_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_buffer->size_x, frame_buffer->size_y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//// set depth texture
	frame_buffer->depth_tex = new Texture();
	glGenTextures(1, &frame_buffer->depth_tex->gl_id);
	glBindTexture(GL_TEXTURE_2D, frame_buffer->depth_tex->gl_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, frame_buffer->size_x, frame_buffer->size_y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

}

void Camera::Save(JSON_Object * deff){

	// Frustum pos
	json_object_set_number(deff, "pos_x", frustum->pos.x);
	json_object_set_number(deff, "pos_y", frustum->pos.y);
	json_object_set_number(deff, "pos_z", frustum->pos.z);

	// Frustum reference
	json_object_set_number(deff, "ref_x", Reference.x);
	json_object_set_number(deff, "ref_y", Reference.y);
	json_object_set_number(deff, "ref_z", Reference.z);

	// Frustum type
	json_object_set_number(deff, "frustum_type", (int)frustum_type);

	// Bools
	json_object_set_boolean(deff, "active", active);
	json_object_set_boolean(deff, "draw_in_UI", draw_in_UI);
	json_object_set_boolean(deff, "draw_frustum", draw_frustum);
	json_object_set_boolean(deff, "draw_depth", draw_depth);

}

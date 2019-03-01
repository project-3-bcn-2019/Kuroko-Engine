#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "MathGeoLib\Math\float4x4.h"
#include "MathGeoLib\Math\float3.h"
#include "MathGeoLib\Geometry\Frustum.h"
#include "MathGeoLib\Geometry\OBB.h"

#include "Parson\parson.h"
#include "Globals.h"

class ComponentCamera;
class FrameBuffer;
class Texture;

#define EXTRA_DIST 3.0f
#define INIT_HOR_FOV 90.0f
#define INIT_VER_FOV 59.0f
#define INIT_N_PLANE 0.5f
#define INIT_F_PLANE 1250.0f
#define INIT_ORT_SIZE 20.0f

enum ViewportDir { VP_RIGHT, VP_LEFT, VP_UP, VP_DOWN, VP_FRONT, VP_BACK, VP_NONE };

struct FrameBuffer
{
	~FrameBuffer();
	Texture* tex = nullptr;
	Texture* depth_tex = nullptr;

	uint size_x = 0;
	uint size_y = 0;
};

class Camera
{
	friend class ModuleCamera3D;
	friend class ComponentCamera;
public:
	Camera(float3 position = float3::zero, float3 reference = float3(0.0f, 0.0f, 5.0f), math::FrustumType frustum_type = math::FrustumType::PerspectiveFrustum);
	~Camera();

	void LookAt(const float3 &Spot);
	void Move(const float3 &Movement);
	void FitToSizeSelectedGeometry(float extra_distance = EXTRA_DIST);
	void LookAtSelectedGeometry();
	void Reset();

	Frustum* getFrustum() const { return frustum; }; 
	FrameBuffer* getFrameBuffer() const { return frame_buffer; };
	ComponentCamera* getParent() const { return attached_to; }; // can be null

	float* GetProjectionMatrix()const;
	float* GetViewMatrix()const;

	void setFrameBuffer(FrameBuffer* fb) { frame_buffer = fb; };
	void setFrustum(Frustum* f) { frustum = f; };
	void setFov(float hor_fov, float ver_fov);
	void setPlaneDistance(float n_plane, float f_plane);
	bool frustumCull(const OBB& obb);  // returns true if inside the frustum

	bool IsViewport();
	ViewportDir getViewportDir();
	std::string getViewportDirString();

	void updateFrustum();
	void initFrameBuffer();

	void Save(JSON_Object* deff);

public:

	float3 X = { 1.0f,0.0f,0.0f };
	float3 Y = { 0.0f,1.0f,0.0f };
	float3 Z = { 0.0f,0.0f,1.0f };
	float3 Reference = { 0.0f,0.0f,5.0f };

	bool active = false;
	bool draw_in_UI = false;
	bool draw_frustum = false;
	bool draw_depth = false;

private:

	ComponentCamera* attached_to = nullptr;  // can be null
	FrameBuffer* frame_buffer = nullptr;
	Frustum* frustum = nullptr;
	math::FrustumType frustum_type = math::FrustumType::PerspectiveFrustum;

};


#endif

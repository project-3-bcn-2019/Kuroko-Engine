#ifndef __MODULE_DEBUG
#define __MODULE_DEBUG

#include "Globals.h"
#include "Module.h"
#include "Color.h"

#include "MathGeoLib\Math\float3.h"
#include "MathGeoLib\Math\float2.h"
#include "MathGeoLib\Geometry\Frustum.h"
#include "MathGeoLib\Math\Quat.h"
#include "MathGeoLib\Geometry\OBB.h"

#include <list>

enum ShapeType { S_UNKNOWN, CUBE, SPHERE, ARROW, AXIS, RAY, FRUSTUM};

struct DebugShape
{
	~DebugShape();
	ShapeType type = S_UNKNOWN;

	float2* lines = nullptr;
	float3* vertices = nullptr;
	float3* normals = nullptr;
	float3* colors = nullptr;

	uint id = 0;
	uint vboId = 0;
	uint iboId = 0;

	uint num_lines = 0;
	uint num_vertices = 0;

	void Draw() const;
	void LoadDataToVRAM();
};

class ModuleDebug : public Module
{
public:
	ModuleDebug(Application* app, bool start_enabled = true) : Module(app, start_enabled) { name = "debug"; };
	~ModuleDebug() { ClearShapes(); };

	void DrawShapes();
	bool Start();
	bool CleanUp();

	uint addArrow(const float3& start_point, const float3& end_point, const Color& color = Red);
	uint addAxis(const float3& position, float length = 1.0f, const Quat& rotation = Quat::identity);
	uint addRay(const float3& start_point, const float3& end_point, const Color& color = Green);
	uint addFrustum(const float3& pos, const Quat& rotation = Quat::identity, FrustumType type = PerspectiveFrustum, float n_plane = 0.5f, float f_plane = 10.0f, float h_fov_or_ortho_width = 90.0f, float v_fov_or_ortho_height = 59.0f, const Color& color = Blue);

	void directDrawFrustum(const Frustum& f);
	void directDrawAxis(const float3& position, const Quat& rotation = Quat::identity, float length = 1.0f);

	void removeShape(uint id);
	void ClearShapes();

private:

	void DrawGrid() const;

public:
	bool draw_grid = true;

private:
	std::list<DebugShape*> shapes;
	uint last_shape_id = 0;
	math::OBB grid_obb;
};
#endif
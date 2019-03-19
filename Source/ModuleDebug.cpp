#include "ModuleDebug.h"
#include "Application.h"
#include "ModuleCamera3D.h"
#include "Camera.h"

#include "MathGeoLib\Geometry\LineSegment.h"

#include "glew-2.1.0\include\GL\glew.h"
#include "SDL\include\SDL_opengl.h"

#include <gl/GL.h>
#include <gl/GLU.h>

bool ModuleDebug::Start()
{
	grid_obb.r = { 20.0f, 0.1f, 20.0f };
	grid_obb.pos = float3::zero;
	grid_obb.axis[0] = float3::unitX;
	grid_obb.axis[1] = float3::unitX;
	grid_obb.axis[2] = float3::unitZ;

	return true;
}

bool ModuleDebug::CleanUp()
{
	for (auto it = shapes.begin(); it != shapes.end(); it++)
		delete *it;

	shapes.clear();
	return true;
}

void ModuleDebug::DrawShapes()
{
	if (draw_grid)	
		DrawGrid();

	for (auto it = App->camera->game_cameras.begin(); it != App->camera->game_cameras.end(); it++)
	{
		if ((*it)->draw_frustum)
			directDrawFrustum(*(*it)->getFrustum());
	}

	for (std::list<DebugShape*>::iterator it = shapes.begin(); it != shapes.end(); it++)
		(*it)->Draw();
}

uint ModuleDebug::addArrow(const float3& start_point, const float3& end_point, const Color& color)
{
	DebugShape* arrow = new DebugShape();
	arrow->type = ARROW;
	float3 aux = end_point - start_point;

	arrow->num_vertices = 6;
	arrow->vertices = new float3[6];

	float3 perp1 = aux.Perpendicular().Normalized() * 0.3f;
	float3 perp2 = aux.AnotherPerpendicular().Normalized() * 0.3f;
	arrow->vertices[0] = start_point;			arrow->vertices[1] = end_point;
	arrow->vertices[2] = (aux* 0.95f) - (perp1); arrow->vertices[3] = (aux* 0.95f) + (perp1);
	arrow->vertices[4] = (aux* 0.95f) - (perp2); arrow->vertices[5] = (aux* 0.95f) + (perp2);
	
	arrow->num_lines = 9;
	arrow->lines = new float2[9];
	arrow->lines[0] = { 0,1 }; arrow->lines[1] = { 2,1 }; arrow->lines[2] = { 3,1 };
	arrow->lines[3] = { 4,1 }; arrow->lines[4] = { 5,1 }; arrow->lines[5] = { 2,4 }; 
	arrow->lines[6] = { 2,5 }; arrow->lines[7] = { 3,5 }; arrow->lines[8] = { 4,3 };

	arrow->colors = new float3[6];
	for (int i = 0; i < 6; i++)
		arrow->colors[i] = { color.r, color.g, color.b };

	arrow->normals = new float3[6];
	aux.Normalize();
	arrow->normals[0] = aux.Neg(); arrow->normals[1] = aux;
	arrow->normals[2] = (arrow->vertices[2] - end_point).Normalized(); arrow->normals[3] = (arrow->vertices[3] - end_point).Normalized();
	arrow->normals[4] = (arrow->vertices[4] - end_point).Normalized(); arrow->normals[5] = (arrow->vertices[5] - end_point).Normalized();

	arrow->id = last_shape_id++;
	arrow->LoadDataToVRAM();

	shapes.push_back(arrow);
	return arrow->id;
}

uint ModuleDebug::addAxis(const float3& position, float length, const Quat& rotation)
{
	float3 X = { length, 0.0f, 0.0f };
	float3 Y = { 0.0f, length, 0.0f };
	float3 Z = { 0.0f, 0.0f, length };

	if (!rotation.Equals(Quat::identity))
	{
		X = rotation * X;
		Y = rotation * Y;
		Z = rotation * Z;
	}

	DebugShape* axis = new DebugShape();
	axis->type = AXIS;

	axis->num_vertices = 4;
	axis->vertices = new float3[4];
	axis->vertices[0] = position; axis->vertices[1] = position + X; axis->vertices[2] = position + Y; axis->vertices[3] = position + Z;

	axis->num_lines = 3;
	axis->lines = new float2[3];
	axis->lines[0] = { 0,1 }; axis->lines[1] = { 0,2 }; axis->lines[2] = { 0,3 };

	axis->colors = new float3[4];
	axis->colors[0] = { 1.0f, 1.0f, 1.0f }; axis->colors[1] = { 1.0f, 0.0f, 0.0f }; 
	axis->colors[2] = { 0.0f, 1.0f, 0.0f }; axis->colors[3] = { 0.0f, 0.0f, 1.0f };

	axis->normals = new float3[4];
	axis->normals[0] = float3::one;			 axis->normals[1] = X.Neg().Normalized();
	axis->normals[2] = Y.Neg().Normalized();  axis->normals[3] = Z.Neg().Normalized();

	axis->id = last_shape_id++;
	axis->LoadDataToVRAM();

	shapes.push_back(axis);
	return axis->id;
}

uint ModuleDebug::addRay(const float3& start_point, const float3& end_point, const Color& color)
{
	DebugShape* ray = new DebugShape();
	ray->type = RAY;

	ray->num_vertices = 2;
	ray->vertices = new float3[2];
	ray->vertices[0] = start_point; ray->vertices[1] = end_point;

	ray->num_lines = 1;
	ray->lines = new float2[1];
	ray->lines[0] = { 0,1 }; 

	ray->colors = new float3[2];
	ray->colors[0] = { color.r, color.g, color.b }; ray->colors[1] = { color.r, color.g, color.b };

	ray->normals = new float3[2];
	ray->normals[0] = (start_point - end_point).Normalized(); ray->normals[1] = (end_point - start_point).Normalized();

	ray->id = last_shape_id++;
	ray->LoadDataToVRAM();

	shapes.push_back(ray);
	return ray->id;
}

uint  ModuleDebug::addFrustum(const float3& pos, const Quat& rotation, FrustumType type, float n_plane, float f_plane, float h_fov_or_ortho_width, float v_fov_or_ortho_height, const Color& color)
{
	Frustum f;
	f.pos = pos;
	f.front = rotation * float3::unitZ;
	f.up	= rotation * float3::unitY;
	f.nearPlaneDistance = n_plane;
	f.farPlaneDistance = f_plane;
	f.type = type;
	if (type == OrthographicFrustum) { f.orthographicHeight = v_fov_or_ortho_height;	f.orthographicWidth = h_fov_or_ortho_width; }
	else							 { f.verticalFov = h_fov_or_ortho_width;			f.horizontalFov = v_fov_or_ortho_height; }
	
	DebugShape* frustum = new DebugShape();
	frustum->type = FRUSTUM;

	frustum->num_vertices = 8;
	frustum->vertices = new float3[8];
	float3 corners[8];
	f.GetCornerPoints(corners);
	for (int i = 0; i < 8; i++)
		frustum->vertices[i] = corners[i];
	
	frustum->num_lines = 12;
	frustum->lines = new float2[12];
	frustum->lines[0] = {4, 6};	frustum->lines[1] = {4, 0};	
	frustum->lines[6] = {2, 6};	frustum->lines[7] = {2, 0};	
	frustum->lines[2] = {5, 1};	frustum->lines[3] = {5, 7};
	frustum->lines[8] = {3, 1};	frustum->lines[9] = {7, 3};
	frustum->lines[4] = {4, 5};	frustum->lines[5] = {2, 3};	
	frustum->lines[10] = { 1, 0 };	frustum->lines[11] = { 7, 6 };

	frustum->normals = new float3[8];
	for (int i = 0; i < 8; i++)
		frustum->normals[i] = (frustum->vertices[i] - f.CenterPoint()).Normalized();

	frustum->colors = new float3[8];
	for (int i = 0; i < 8; i++)
		frustum->colors[i] = { color.r, color.g, color.b };

	frustum->id = last_shape_id++;
	frustum->LoadDataToVRAM();

	shapes.push_back(frustum);
	return frustum->id;

}

void ModuleDebug::directDrawFrustum(const Frustum& f)
{
	glLineWidth(1.5f);

	glColor3f(0.0f, 0.7f, 0.0f);
	glBegin(GL_LINES);
	
	for (int i = 0; i < 12; i++)
	{
		float3 a = f.Edge(i).a;
		float3 b = f.Edge(i).b;
		glVertex3f(f.Edge(i).a.x, f.Edge(i).a.y, f.Edge(i).a.z);
		glVertex3f(f.Edge(i).b.x, f.Edge(i).b.y, f.Edge(i).b.z);
	}

	glEnd();
	glColor3f(1.0f, 1.0f, 1.0f);

	glLineWidth(1.0f);
}

void ModuleDebug::directDrawAxis(const float3& position, const Quat& rotation, float length)
{
	glLineWidth(1.5f);

	glBegin(GL_LINES);

	float3 X = rotation * float3::unitX;
	float3 Y = rotation * float3::unitY;
	float3 Z = rotation * float3::unitZ;

	glColor3f(1.0f, 0.0f, 0.0f);

	glVertex3f(position.x, position.y, position.z);
	glVertex3f(position.x + X.x, position.y + X.y, position.z + X.z);

	glColor3f(0.0f, 1.0f, 0.0f);

	glVertex3f(position.x, position.y, position.z);
	glVertex3f(position.x + Y.x, position.y + Y.y, position.z + Y.z);

	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(position.x, position.y, position.z);
	glVertex3f(position.x + Z.x, position.y + Z.y, position.z + Z.z);

	glEnd();
	glColor3f(1.0f, 1.0f, 1.0f);

	glLineWidth(1.0f);
}

void ModuleDebug::DrawGrid() const
{
	if (!App->camera->current_camera->frustumCull(grid_obb))
		return;

	glLineWidth(1.0f);
	glColor3f(0.5f, 0.5f, 0.5f);
	glBegin(GL_LINES);

	float d = 20.0f;

	for (float i = -d; i <= d; i += 1.0f)
	{
		glVertex3f(i, 0.0f, -d);
		glVertex3f(i, 0.0f, d);
		glVertex3f(-d, 0.0f, i);
		glVertex3f(d, 0.0f, i);
	}

	glColor3f(1.0f, 1.0f, 1.0f);
	glEnd();

}

void ModuleDebug::removeShape(uint id)
{
	DebugShape* to_erase = nullptr;

	for (auto it = shapes.begin(); it != shapes.end(); it++)
	{
		if ((*it)->id == id)
		{
			to_erase = *it;
			break;
		}
	}

	shapes.remove(to_erase);
}

void ModuleDebug::ClearShapes()
{
	for (auto it = shapes.begin(); it != shapes.end(); it++)
		delete *it;

	shapes.clear();
}


void DebugShape::Draw() const
{
	// bind VBOs before drawing
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

	// enable vertex arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	bool face_cull_enabled = glIsEnabled(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);

	glLineWidth(3.0f);

	size_t Offset = sizeof(float3) * num_vertices;

	// specify vertex arrays with their offsets
	glVertexPointer(3, GL_FLOAT, 0, (void*)0);
	glNormalPointer(GL_FLOAT, 0, (void*)Offset);
	glColorPointer(3, GL_FLOAT, 0, (void*)(Offset * 2));

	glDrawElements(GL_LINES, num_lines * 2, GL_UNSIGNED_INT, NULL);

	glLineWidth(1.0f);

	if(face_cull_enabled) glEnable(GL_CULL_FACE);
		
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	// unbind VBOs
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void DebugShape::LoadDataToVRAM()
{
	// create VBOs
	glGenBuffers(1, &vboId);    // for vertex buffer
	glGenBuffers(1, &iboId);    // for index buffer

	size_t vSize = sizeof(float3) * num_vertices;

	// copy vertex attribs data to VBO
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, vSize + vSize + vSize, 0, GL_STATIC_DRAW); // reserve space
	glBufferSubData(GL_ARRAY_BUFFER, 0, vSize, vertices);                  // copy verts at offset 0
	glBufferSubData(GL_ARRAY_BUFFER, vSize, vSize, normals);               // copy norms after verts
	glBufferSubData(GL_ARRAY_BUFFER, vSize + vSize, vSize, colors);          // copy cols after norms
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// copy index data to VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float2) * num_lines, lines, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

DebugShape::~DebugShape()
{
	if(iboId != 0)
		glDeleteBuffers(1, &iboId);
	if (vboId != 0)
		glDeleteBuffers(1, &vboId);

	if (vertices)	delete vertices;
	if (lines)		delete lines;
	if (normals)		delete normals;
	if (colors)		delete colors;
}
#ifndef _MESH
#define _MESH
#include "Globals.h"
#include "Color.h"
#include <string>

#include "MathGeoLib\Math\float3.h"
#include "MathGeoLib\Math\float2.h"

class aiMesh;
class aiScene;
class Material;

enum PrimitiveTypes
{
	Primitive_Point,
	Primitive_Line,
	Primitive_Plane,
	Primitive_Cube,
	Primitive_Sphere,
	Primitive_Cylinder,
	Primitive_None
};

struct Tri{
	uint v1 = 0; uint v2 = 0; uint v3 = 0;
	void set(uint v1, uint v2, uint v3) { this->v1 = v1; this->v2 = v2; this->v3 = v3; }
};

class Mesh {
	
	friend class Skybox;
public:
	Mesh(const aiMesh& mesh, const aiScene& scene, const char* file_name = "");
	Mesh(PrimitiveTypes primitive);
	// Maybe a "MeshDef" could be created?
	Mesh(float3* vertices, Tri* tris, float3* normals, float3* colors, float2* tex_coords, uint num_vertices, uint num_tris, const float3& centroid = float3::zero); //Used to load own file
	~Mesh();

	void Draw(Material* mat, bool draw_as_selected = false) const;
	void DrawNormals() const;

	void getData(uint& vert_num, uint& poly_count, bool& has_normals, bool& has_colors, bool& has_texcoords) const;
	float3 getHalfSize() const { return half_size; }
	float3 getCentroid() const { return centroid; }
	uint getId() const { return id; };

	const float3* getVertices()		{ return vertices; }
	const Tri* getTris()			{ return tris; }
	const float3* getNormals()		{ return normals; }
	const float3* getColors()		{ return colors; }
	const float2* getTexCoords()	{ return tex_coords; }
	const char* getName()			{ return mesh_name.c_str(); }         
	void setName(const char* name)	{ mesh_name = name; }
	uint getNumVertices() { return num_vertices; }
	uint getNumTris() { return num_tris; }

private:

	void LoadDataToVRAM();
	void BuildCube(float3& size = (float3)float3::one);
	void BuildPlane(float sx = 1.0f, float sy = 1.0f);
	void BuildSphere(float radius = 1.0f, float sectorCount = 12.0f, float stackCount = 24.0f);
	void BuildCylinder(float radius = 1.0f, float length = 3.0f, int steps = 30);
	bool LoadFromAssimpMesh(const aiMesh& mesh, const aiScene& scene);
	void ClearData();
	void calculateCentroidandHalfsize();

private:

	const uint id = 0;
	uint iboId = 0;
	uint vboId = 0;
	std::string mesh_name; //"CUBE", "PLANE", "SHPERE" and "CYLINDER" are primitives, don't use models with this name

	uint num_vertices = 0;
	uint num_tris = 0;
	bool imported_normals = false;
	bool imported_colors = false;
	bool imported_tex_coords = false;

	float3* vertices = nullptr;
	Tri* tris = nullptr;
	float3* normals = nullptr;
	float3* colors = nullptr;
	float2* tex_coords = nullptr;

	float3 half_size = float3::zero;
	float3 centroid = float3::zero;

public:
	Color tint_color = White;
};
#endif
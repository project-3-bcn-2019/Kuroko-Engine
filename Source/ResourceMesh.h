#ifndef __MESH_RESOURCE
#define __MESH_RESOURCE

#include "Resource.h"
class Mesh;
class ResourceMesh :
	public Resource {
public:
	ResourceMesh(resource_deff deff);
	~ResourceMesh();
	void LoadToMemory();
	void UnloadFromMemory();
public:
	Mesh* mesh = nullptr;
	std::string Parent3dObject;
};

#endif
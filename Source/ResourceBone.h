#ifndef __RESOURCEBONE_H__
#define __RESOURCEBONE_H__

#include "Resource.h"

#include "MathGeoLib/MathGeoLib.h"

struct Weight {

	uint VertexID;
	float weight;

};

class ComponentMesh;

class ResourceBone : public Resource
{
public:

	ResourceBone(resource_deff deff);
	~ResourceBone();

	void LoadToMemory();
	bool LoadBone();
	void UnloadFromMemory();

public:

	std::string Parent3dObject;

	uint numWeights;

	float4x4 Offset;
	float* pos;
	float* rot;
	float* scale;

	Weight* weights;
	uint boneMeshUID;
};

#endif // !__RESOURCEBONE_H__

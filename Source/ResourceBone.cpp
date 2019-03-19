#include "ResourceBone.h"
#include "Application.h"

ResourceBone::ResourceBone(resource_deff deff):Resource(deff)
{
	Parent3dObject = deff.Parent3dObject;
}

ResourceBone::~ResourceBone()
{
}

void ResourceBone::LoadToMemory()
{
	LoadBone();
}

bool ResourceBone::LoadBone()
{
	bool ret = true;

	//Get the buffer
	char* buffer = App->fs.ImportFile(binary.c_str());
	char* cursor = buffer;

	uint bytes = sizeof(uint);
	memcpy(&numWeights, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(float) * 3;
	pos = new float[3];
	memcpy(pos, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(float) * 4;
	rot = new float[4];
	memcpy(rot, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(float) * 3;
	scale = new float[3];
	memcpy(scale, cursor, bytes);
	cursor += bytes;

	Offset = float4x4::FromTRS({ pos[0],pos[1],pos[2] }, { rot[0],rot[1],rot[2],rot[3] }, { scale[0],scale[1],scale[2] });

	if (numWeights > 0)
	{
		weights = new Weight[numWeights];

		for (int i = 0; i < numWeights; i++)
		{
			bytes = sizeof(uint);
			memcpy(&weights[i].VertexID, cursor, bytes);
			cursor += bytes;

			bytes = sizeof(float);
			memcpy(&weights[i].weight, cursor, bytes);
			cursor += bytes;
		}
	}

	RELEASE_ARRAY(buffer);

	return true;
}

void ResourceBone::UnloadFromMemory()
{
	RELEASE_ARRAY(pos);
	RELEASE_ARRAY(rot);
	RELEASE_ARRAY(scale);
	RELEASE_ARRAY(weights);
}

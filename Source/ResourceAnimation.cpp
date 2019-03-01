#include "ResourceAnimation.h"
#include "Application.h"
#include "Globals.h"
#include "ModuleTimeManager.h"

ResourceAnimation::ResourceAnimation(resource_deff deff):Resource(deff)
{
	Parent3dObject = deff.Parent3dObject;
}

ResourceAnimation::~ResourceAnimation()
{
}

void ResourceAnimation::LoadToMemory()
{
	LoadAnimation();
}

void ResourceAnimation::UnloadFromMemory()
{

}

bool ResourceAnimation::LoadAnimation()
{
	bool ret = false;

	//Get the buffer
	char* buffer = App->fs.ImportFile(binary.c_str());
	char* cursor = buffer;

	//Load ranges
	uint ranges[3];
	uint bytes = sizeof(ranges);
	memcpy(ranges, cursor, bytes);
	cursor += bytes;

	ticks = ranges[0];
	ticksXsecond = ranges[1];
	numBones = ranges[2];

	if (numBones > 0)
	{
		boneTransformations = new BoneTransform[numBones];

		//Loading Bones
		for (int i = 0; i < numBones; i++)
		{
			//Loading Ranges
			uint boneRanges[4];
			uint bytes = sizeof(boneRanges);
			memcpy(boneRanges, cursor, bytes);
			cursor += bytes;

			boneTransformations[i].numPosKeys = boneRanges[0];
			boneTransformations[i].numScaleKeys = boneRanges[1];
			boneTransformations[i].numRotKeys = boneRanges[2];

			//Loading Name
			bytes = boneRanges[3];
			char* auxName = new char[bytes];
			memcpy(auxName, cursor, bytes);
			boneTransformations[i].NodeName = auxName;
			boneTransformations[i].NodeName = boneTransformations[i].NodeName.substr(0, bytes);
			RELEASE_ARRAY(auxName);
			cursor += bytes;

			//Loading Pos Time
			bytes = boneTransformations[i].numPosKeys * sizeof(double);
			boneTransformations[i].PosKeysTimes = new double[boneTransformations[i].numPosKeys];
			memcpy(boneTransformations[i].PosKeysTimes, cursor, bytes);
			cursor += bytes;
			//Loading Pos Values
			bytes = boneTransformations[i].numPosKeys * sizeof(float) * 3;
			boneTransformations[i].PosKeysValues = new float[boneTransformations[i].numPosKeys * 3];
			memcpy(boneTransformations[i].PosKeysValues, cursor, bytes);
			cursor += bytes;

			//Loading Scale Time
			bytes = boneTransformations[i].numScaleKeys * sizeof(double);
			boneTransformations[i].ScaleKeysTimes = new double[boneTransformations[i].numScaleKeys];
			memcpy(boneTransformations[i].ScaleKeysTimes, cursor, bytes);
			cursor += bytes;
			//Loading Scale Values
			bytes = boneTransformations[i].numScaleKeys * sizeof(float) * 3;
			boneTransformations[i].ScaleKeysValues = new float[boneTransformations[i].numScaleKeys * 3];
			memcpy(boneTransformations[i].ScaleKeysValues, cursor, bytes);
			cursor += bytes;

			//Loading Rotation Time
			bytes = boneTransformations[i].numRotKeys * sizeof(double);
			boneTransformations[i].RotKeysTimes = new double[boneTransformations[i].numRotKeys];
			memcpy(boneTransformations[i].RotKeysTimes, cursor, bytes);
			cursor += bytes;
			//Loading Rotation Values
			bytes = boneTransformations[i].numRotKeys * sizeof(float) * 4;
			boneTransformations[i].RotKeysValues = new float[boneTransformations[i].numRotKeys * 4];
			memcpy(boneTransformations[i].RotKeysValues, cursor, bytes);
			cursor += bytes;
		}

		ret = true;
	}

	RELEASE_ARRAY(buffer);

	return ret;
}

void ResourceAnimation::resetFrames()
{
	for (int i = 0; i < numBones; i++)
	{
		boneTransformations[i].currentPosIndex = 0;
		boneTransformations[i].currentRotIndex = 0;
		boneTransformations[i].currentScaleIndex = 0;
	}
}

float ResourceAnimation::getDuration() const
{
	return ticks / ticksXsecond;
}

BoneTransform::~BoneTransform()
{
	RELEASE_ARRAY(PosKeysValues);
	RELEASE_ARRAY(PosKeysTimes);

	RELEASE_ARRAY(ScaleKeysValues);
	RELEASE_ARRAY(ScaleKeysTimes);

	RELEASE_ARRAY(RotKeysValues);
	RELEASE_ARRAY(RotKeysTimes);
}

bool BoneTransform::calcCurrentIndex(float time, bool test)
{
	bool ret = false;

	if ((App->time->getGameState() != GameState::PLAYING && !test) || currentPosIndex == -1 || currentRotIndex == -1 || currentScaleIndex == -1 ||
		nextPosIndex == -1 || nextRotIndex == -1 || nextScaleIndex == -1)
	{
		currentPosIndex = currentRotIndex = currentScaleIndex = 0;
		nextPosIndex = nextRotIndex = nextScaleIndex = 1;
		return true;
	}

	for (int i = 0; i < numPosKeys; i++)
	{
		if (PosKeysTimes[i] <= time && (i + 1 >= numPosKeys || PosKeysTimes[i + 1] > time))
		{
			currentPosIndex = i;

			nextPosIndex = currentPosIndex + 1;

			if (nextPosIndex >= numPosKeys && numPosKeys > 1)
			{
				nextPosIndex = 0;
			}


			ret = true;
			break;
		}
	}
	for (int i = 0; i < numRotKeys; i++)
	{
		if (RotKeysTimes[i] <= time && (i + 1 >= numRotKeys || RotKeysTimes[i + 1] > time))
		{
			currentRotIndex = i;

			nextRotIndex = currentRotIndex + 1;

			if (nextRotIndex >= numRotKeys && numRotKeys > 1)
			{
				nextRotIndex = 0;
			}

			ret = true;
			break;
		}
	}
	for (int i = 0; i < numScaleKeys; i++)
	{
		if (ScaleKeysTimes[i] <= time && (i + 1 >= numScaleKeys || ScaleKeysTimes[i + 1] > time))
		{
			currentScaleIndex = i;

			nextScaleIndex = currentScaleIndex + 1;

			if (nextScaleIndex >= numScaleKeys && numScaleKeys > 1)
			{
				nextScaleIndex = 0;
			}

			ret = true;
			break;
		}
	}

	return ret;
}

void BoneTransform::calcTransfrom(float time, bool interpolation)
{
	float tp, ts, tr;

	tp = ts = tr = 0.0f;

	float3 position_1 = { PosKeysValues[currentPosIndex * 3], PosKeysValues[currentPosIndex * 3 + 1], PosKeysValues[currentPosIndex * 3 + 2] };
	Quat rotation_1 = { RotKeysValues[currentRotIndex * 4], RotKeysValues[currentRotIndex * 4 + 1], RotKeysValues[currentRotIndex * 4 + 2], RotKeysValues[currentRotIndex * 4 + 3] };
	float3 scale_1 = { ScaleKeysValues[currentScaleIndex * 3], ScaleKeysValues[currentScaleIndex * 3 + 1], ScaleKeysValues[currentScaleIndex * 3 + 2] };

	float3 position_2 = { PosKeysValues[nextPosIndex * 3], PosKeysValues[nextPosIndex * 3 + 1], PosKeysValues[nextPosIndex * 3 + 2] };
	Quat rotation_2 = { RotKeysValues[nextRotIndex * 4], RotKeysValues[nextRotIndex * 4 + 1], RotKeysValues[nextRotIndex * 4 + 2], RotKeysValues[nextRotIndex * 4 + 3] };
	float3 scale_2 = { ScaleKeysValues[nextScaleIndex * 3], ScaleKeysValues[nextScaleIndex * 3 + 1], ScaleKeysValues[nextScaleIndex * 3 + 2] };


	tp = ((time - PosKeysTimes[currentPosIndex]) / (PosKeysTimes[nextPosIndex] - PosKeysTimes[currentPosIndex]));
	tr = ((time - RotKeysTimes[currentRotIndex]) / (RotKeysTimes[nextRotIndex] - RotKeysTimes[currentRotIndex]));
	ts = ((time - ScaleKeysTimes[currentScaleIndex]) / (ScaleKeysTimes[nextScaleIndex] - ScaleKeysTimes[currentScaleIndex]));

	tp = (tp < 0) ? 0 : tp;
	tr = (tr < 0) ? 0 : tr;
	ts = (ts < 0) ? 0 : ts;

	float3 position = position_1.Lerp(position_2, tp);
	Quat rotation = rotation_1.Slerp(rotation_2, tr);
	float3 scale = scale_1.Lerp(scale_2, ts);

	if (interpolation)
	{
		lastTransform.Set(float4x4::FromTRS(position, rotation, scale));
	}
	else
	{
		lastTransform.Set(float4x4::FromTRS(position_1, rotation_1, scale_1));
	}
}

void BoneTransform::smoothBlending(const float4x4& blendtrans, float time)
{
	float3 position_1;
	Quat rotation_1;
	float3 scale_1;

	lastTransform.Decompose(position_1, rotation_1, scale_1);

	float3 position_2;
	Quat rotation_2;
	float3 scale_2;

	blendtrans.Decompose(position_2, rotation_2, scale_2);

	float3 finalpos = position_1.Lerp(position_2, time);
	Quat finalrot = rotation_1.Slerp(rotation_2, time);
	float3 finalscale = scale_1.Lerp(scale_2, time);

	lastTransform.Set(float4x4::FromTRS(finalpos, finalrot, finalscale));
}

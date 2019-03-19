#pragma once
#include "Resource.h"
class ResourceAudio :
	public Resource
{
public:
	ResourceAudio(resource_deff deff);
	~ResourceAudio();

	std::string audio_name;
};


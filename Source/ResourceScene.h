#pragma once
#include "Resource.h"
class ResourceScene :
	public Resource {
public:
	ResourceScene(resource_deff deff);
	~ResourceScene();

	std::string scene_name;
};


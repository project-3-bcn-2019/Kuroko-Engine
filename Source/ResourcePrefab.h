#pragma once
#include "Resource.h"
class ResourcePrefab :
	public Resource {
public:
	ResourcePrefab(resource_deff deff);
	~ResourcePrefab();

	std::string prefab_name;
};


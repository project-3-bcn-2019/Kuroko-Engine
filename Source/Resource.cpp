#include "Resource.h"
#include "Parson\parson.h"
#include "Application.h"


Resource::Resource(resource_deff deff) {
	uuid = deff.uuid;
	type = deff.type;
	binary = deff.binary;
	asset = deff.asset;
}

void Resource::LoadToMemory()
{
}

void Resource::UnloadFromMemory()
{
}

bool Resource::IsLoaded()
{
	return loaded_in_memory;
}
#include "ResourcePrefab.h"
#include "Application.h"
#include "ModuleScene.h"


ResourcePrefab::ResourcePrefab(resource_deff deff): Resource(deff) {
	prefab_name = deff.asset;
	App->fs.getFileNameFromPath(prefab_name);
}


ResourcePrefab::~ResourcePrefab() {
}

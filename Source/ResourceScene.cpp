#include "ResourceScene.h"
#include "Application.h"
#include "ModuleScene.h"


ResourceScene::ResourceScene(resource_deff deff): Resource(deff) {
	scene_name = deff.asset;
	App->fs.getFileNameFromPath(scene_name);
}


ResourceScene::~ResourceScene() {
}


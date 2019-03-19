#include "ResourceTexture.h"
#include "Material.h"
#include "Application.h"
#include "ModuleImporter.h"
#include "Applog.h"
#include "Material.h"


ResourceTexture::ResourceTexture(resource_deff deff): Resource(deff) {

}


ResourceTexture::~ResourceTexture() {

}

void ResourceTexture::LoadToMemory() {
	texture = App->importer->LoadTextureFromLibrary(binary.c_str());

	if(!texture)
		app_log->AddLog("Error loading binary %s", binary.c_str());
	else
		loaded_in_memory = true;

}

void ResourceTexture::UnloadFromMemory() {
	if (!texture)
		return;
	delete texture;
	texture = nullptr;
	loaded_in_memory = false;
}

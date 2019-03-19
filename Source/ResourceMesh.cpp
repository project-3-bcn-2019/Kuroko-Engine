#include "ResourceMesh.h"
#include "Mesh.h"
#include "Application.h"
#include "ModuleImporter.h"
#include "Applog.h"



ResourceMesh::ResourceMesh(resource_deff deff): Resource(deff) {
	Parent3dObject = deff.Parent3dObject;
}


ResourceMesh::~ResourceMesh() {
}

void ResourceMesh::LoadToMemory() {
	mesh = App->importer->ExportMeshFromKR(binary.c_str());

	if (!mesh)
		app_log->AddLog("Error loading binary %s", binary.c_str());
	else
		loaded_in_memory = true;
}

void ResourceMesh::UnloadFromMemory() {
	if (!mesh)
		return;
	delete mesh;
	mesh = nullptr;
	loaded_in_memory = false;
}

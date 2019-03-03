#ifndef __MODULE_EXPORTER
#define __MODULE_EXPORTER

#include "Module.h"

class ModuleExporter : public Module
{
public:
	ModuleExporter(Application* app, bool start_enabled = true);
	~ModuleExporter();

	bool Init(const JSON_Object* config);
	bool CleanUp();

	bool CreateBuild(const char* path, const char* name);

private:
	void AssetsToLibraryJSON();
	JSON_Value* GetAssetFolderUUIDs(const char* path);
	
public:

};

#endif
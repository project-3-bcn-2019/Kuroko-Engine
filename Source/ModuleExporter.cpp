#include "Application.h"
#include "ModuleExporter.h"
#include "GameObject.h"
#include "Globals.h"
#include "Applog.h"


ModuleExporter::ModuleExporter(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	name = "exporter";
}

ModuleExporter::~ModuleExporter() {}


bool ModuleExporter::Init(const JSON_Object* config)
{
	return true;
}

bool ModuleExporter::CleanUp()
{
	return true;
}

bool ModuleExporter::CreateBuild(const char* path, const char* name)
{
	bool ret = App->fs.ExistisFile("../Release/Project-Atlas.exe");

	if (ret)
	{
		AssetsToLibraryJSON();
		std::string fullPath = path;
		fullPath += "\\";
		fullPath += name;
		fullPath += "\\";
		CreateDirectory(fullPath.c_str(), NULL);
		std::string library = fullPath + "Library\\";
		CreateDirectory(library.c_str(), NULL);
		App->fs.CopyFolder("Library\\*", library.c_str(), true);
		std::list<const char*> excludedFiles;
		excludedFiles.push_back("memory.log");
		App->fs.CopyFolder("..\\Game\\*", fullPath.c_str(), false, &excludedFiles);
		App->fs.copyFileTo("../Release/Project-Atlas.exe", NO_LIB, ".exe", fullPath + name);
	}

	return ret;
}

void ModuleExporter::AssetsToLibraryJSON()
{
	JSON_Value* json = json_value_init_object();

	json_object_set_value(json_object(json), "Meshes", GetAssetFolderUUIDs("\\Assets\\Meshes\\*"));
	json_object_set_value(json_object(json), "Prefabs", GetAssetFolderUUIDs("\\Assets\\Prefabs\\*"));
	json_object_set_value(json_object(json), "Scenes", GetAssetFolderUUIDs("\\Assets\\Scenes\\*"));
	json_object_set_value(json_object(json), "Scripts", GetAssetFolderUUIDs("\\Assets\\Scripts\\*"));
	json_object_set_value(json_object(json), "Sounds", GetAssetFolderUUIDs("\\Assets\\Sounds\\*"));
	json_object_set_value(json_object(json), "Textures", GetAssetFolderUUIDs("\\Assets\\Textures\\*"));
	json_object_set_value(json_object(json), "UI", GetAssetFolderUUIDs("\\Assets\\UI\\*"));

	std::string outpath = "Library\\assetsUUIDs.json";
	json_serialize_to_file_pretty(json, outpath.c_str());
	json_value_free(json);
}

JSON_Value* ModuleExporter::GetAssetFolderUUIDs(const char* path)
{
	JSON_Value* object = json_value_init_object();

	char folderPath[256];
	GetCurrentDirectory(256, folderPath);
	std::string fullPath = folderPath;
	fullPath += path;

	WIN32_FIND_DATA file;
	HANDLE search_handle = FindFirstFile(fullPath.c_str(), &file);
	if (search_handle)
	{
		do
		{
			if (file.dwFileAttributes && FILE_ATTRIBUTE_DIRECTORY)
			{
				// Check type of flie
				std::string fileName = file.cFileName, extension = file.cFileName, filePath = fullPath;
				App->fs.getExtension(extension);

				if (extension == ".meta")
				{
					filePath.pop_back();
					filePath += fileName;
					App->fs.getFileNameFromPath(fileName);
					App->fs.getFileNameFromPath(fileName);

					JSON_Value* json = json_parse_file(filePath.c_str());
					if (json)
					{
						JSON_Object* json_obj = json_value_get_object(json);
						uint resourceUUID = json_object_get_number(json_obj, "resource_uuid");
						json_object_set_number(json_object(object), fileName.c_str(), resourceUUID);
					}
					else
						app_log->AddLog("Couldn't load %s, no value", filePath);

					json_value_free(json);
				}
			}
		} while (FindNextFile(search_handle, &file));
		FindClose(search_handle);
	}

	return object;
}
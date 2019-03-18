#include "Application.h"
#include "ModuleExporter.h"
#include "GameObject.h"
#include "Globals.h"
#include "Applog.h"
#include "ModuleUI.h"

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
		std::list<const char*> excludedFiles;
		excludedFiles.push_back("Scenes");
		App->fs.CopyFolder("Library\\*", library.c_str(), true, &excludedFiles);
		excludedFiles.clear();
		excludedFiles.push_back("memory.log");
		App->fs.CopyFolder("..\\Game\\*", fullPath.c_str(), false, &excludedFiles); // Copy files in Game folder (not recursively)
		App->fs.copyFileTo("../Release/Project-Atlas.exe", NO_LIB, ".exe", fullPath + name); // Copy release .exe
		CreateDirectory((fullPath + "Library\\Scenes\\").c_str(), NULL);
		std::list<resource_deff> build_scenes = App->gui->GetBuildScenes(); // Get selected scenes to copy them
		for (auto it = build_scenes.begin(); it != build_scenes.end(); it++) {
			resource_deff scene_deff = (*it);
			App->fs.copyFileTo(scene_deff.binary.c_str(), NO_LIB, ".scene", fullPath + "Library\\Scenes\\" + std::to_string(scene_deff.uuid));
		}

		CreateDirectory((fullPath + SCRIPTINGAPI_FOLDER).c_str(), NULL);
		App->fs.CopyFolder("ScriptingAPI\\*", (fullPath + SCRIPTINGAPI_FOLDER).c_str(), false); // Copy scripts
		CreateDirectory((fullPath + FONTS_FOLDER).c_str(), NULL);
		App->fs.CopyFolder("Fonts\\*", (fullPath + FONTS_FOLDER).c_str(), true); // Copy fonts

		CreateDirectory((fullPath + SETTINGS_FOLDER).c_str(), NULL);
		JSON_Value* config_value = json_parse_file(App->config_file_name.c_str());
		JSON_Object* config = json_value_get_object(config_value);
		json_object_set_boolean(config, "is_game", true);
		json_object_set_number(config, "main_scene", App->gui->getMainScene());
		json_serialize_to_file_pretty(config_value, (fullPath + App->config_file_name).c_str());
		json_value_free(config_value);
		//Add default scene
	}

	return ret;
}

void ModuleExporter::AssetsToLibraryJSON()
{
	JSON_Value* json = json_value_init_object();

	json_object_set_value(json_object(json), "Prefabs", GetAssetFolderUUIDs("\\Assets\\Prefabs\\*"));
	json_object_set_value(json_object(json), "Scenes", GetAssetFolderUUIDs("\\Assets\\Scenes\\*"));
	json_object_set_value(json_object(json), "Scripts", GetAssetFolderUUIDs("\\Assets\\Scripts\\*"));
	json_object_set_value(json_object(json), "Audio", GetAssetFolderUUIDs("\\Assets\\Audio\\*"));
	json_object_set_value(json_object(json), "UI", GetAssetFolderUUIDs("\\Assets\\UI\\*"));
	json_object_set_value(json_object(json), "Shaders", GetAssetFolderUUIDs("\\Assets\\Shaders\\*"));
	json_object_set_value(json_object(json), "AnimationGraphs", GetAssetFolderUUIDs("\\Assets\\AnimationGraphs\\*"));
	json_object_set_value(json_object(json), "Textures", GetAssetFolderUUIDs("\\Assets\\Textures\\*"));
	Get3dObjectsUUIDs("\\Assets\\Meshes\\*", json); // Get UUIDs for Meshes, Animations and Bones from 3dObjets

	std::string outpath = "Library\\assetsUUIDs.json";
	json_serialize_to_file_pretty(json, outpath.c_str());
	json_value_free(json);
}

JSON_Value* ModuleExporter::GetAssetFolderUUIDs(const char* path)
{
	JSON_Value* object = json_value_init_array();

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
					std::string fileExtension = fileName;
					App->fs.getExtension(fileExtension);
					App->fs.getFileNameFromPath(fileName);

					JSON_Value* json = json_parse_file(filePath.c_str());
					if (json)
					{
						JSON_Object* json_obj = json_value_get_object(json);
						JSON_Value* res = json_value_init_object();
						uint resourceUUID = json_object_get_number(json_obj, "resource_uuid");
						json_object_set_string(json_object(res), "name", fileName.c_str());
						json_object_set_string(json_object(res), "extension", fileExtension.c_str());
						json_object_set_number(json_object(res), "uuid", resourceUUID);
						json_array_append_value(json_array(object), res);
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

void ModuleExporter::Get3dObjectsUUIDs(const char* path, JSON_Value* json_value)
{
	JSON_Value* meshes = json_value_init_array();
	JSON_Value* animations = json_value_init_array();
	JSON_Value* bones = json_value_init_array();

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
						std::string binary_path = json_object_get_string(json_obj, "binary_path");

						JSON_Value* json_3dObj = json_parse_file(binary_path.c_str());
						if (json_3dObj)
						{
							JSON_Object* object_3dObj = json_value_get_object(json_3dObj);
							JSON_Array* gameObjects = json_object_get_array(object_3dObj, "Game Objects");
							for (int i = 0; i < json_array_get_count(gameObjects); i++)
							{
								JSON_Object* curr_GO = json_array_get_object(gameObjects, i);
								JSON_Array* comps = json_object_get_array(curr_GO, "Components");
								for (int j = 0; j < json_array_get_count(comps); j++)
								{
									JSON_Object* curr_comp = json_array_get_object(comps, j);
									std::string c_type = json_object_get_string(curr_comp, "type");
									c_type = '.' + c_type;
									if (c_type == ".mesh")
									{
 										const char* c_name = json_object_get_string(curr_comp, "mesh_name");
										uint c_uuid = json_object_get_number(curr_comp, "mesh_resource_uuid");
										JSON_Value* res = json_value_init_object();
										json_object_set_string(json_object(res), "name", c_name);
										json_object_set_string(json_object(res), "extension", c_type.c_str());
										json_object_set_number(json_object(res), "uuid", c_uuid);
										json_array_append_value(json_array(meshes), res);
									}
									if (c_type == ".animation")
									{
										const char* c_name = json_object_get_string(curr_comp, "animation_name");
										uint c_uuid = json_object_get_number(curr_comp, "animation_resource_uuid");
										JSON_Value* res = json_value_init_object();
										json_object_set_string(json_object(res), "name", c_name);
										json_object_set_string(json_object(res), "extension", c_type.c_str());
										json_object_set_number(json_object(res), "uuid", c_uuid);
										json_array_append_value(json_array(animations), res);
									}
									if (c_type == ".bone")
									{
										const char* c_name = json_object_get_string(curr_comp, "bone_name");
										uint c_uuid = json_object_get_number(curr_comp, "bone_resource_uuid");
										JSON_Value* res = json_value_init_object();
										json_object_set_string(json_object(res), "name", c_name);
										json_object_set_string(json_object(res), "extension", c_type.c_str());
										json_object_set_number(json_object(res), "uuid", c_uuid);
										json_array_append_value(json_array(bones), res);
									}
								}
							}
						}
					}
					else
						app_log->AddLog("Couldn't load %s, no value", filePath);
					
					json_value_free(json);
				}
			}
		} while (FindNextFile(search_handle, &file));
		FindClose(search_handle);
	}

	json_object_set_value(json_object(json_value), "Meshes", meshes);
	json_object_set_value(json_object(json_value), "Animations", animations);
	json_object_set_value(json_object(json_value), "Bones", bones);
}
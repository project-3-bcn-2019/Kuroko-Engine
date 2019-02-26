#ifndef _FILE_SYSTEM_
#define _FILE_SYSTEM_

#include "Resource.h"


#define OWN_MESH_EXTENSION ".kr"
#define JSON_EXTENSION ".json"
#define DDS_EXTENSION ".dds"
#define META_EXTENSION ".meta"
#define SCENE_EXTENSION ".scene"
#define PREFAB_EXTENSION ".prefab"

#define LIBRARY_FOLDER "Library\\"
#define MESHES_FOLDER "Library\\Meshes\\"
#define TEXTURES_FOLDER "Library\\Textures\\"
#define OBJECTS_FOLDER "Library\\3dObjects\\"
#define SCRIPTS_FOLDER "Library\\Scripts\\"


#define PREFABS_FOLDER "Library\\Prefabs\\"
#define SCENES_FOLDER "Library\\Scenes"

#define SETTINGS_FOLDER "Settings\\"
#define USER_PREFABS_FOLDER "Assets\\Prefabs\\"
#define USER_SCENES_FOLDER "Assets\\Scenes\\"
#define ASSETS_FOLDER "Assets\\"

enum lib_dir {
	LIBRARY_MESHES,
	LIBRARY_TEXTURES,
	LIBRARY_3DOBJECTS,
	LIBRARY_PREFABS,
	LIBRARY_SCENES,
	LIBRARY_SCRIPTS,
	SETTINGS,
	ASSETS,
	ASSETS_SCENES,
	ASSETS_PREFABS,
	NO_LIB
};

class FileSystem {
public:
	FileSystem() {};
	~FileSystem() {};

public:
	void ExportBuffer(char* data,int size, const char* file_name, lib_dir lib = NO_LIB, const char* extension = ""); // get buffer from a file
	char* ImportFile(const char* file_name); // recieve buffer from a file

	std::string GetFileString(const char* file_name);
	void SetFileString(const char* file_name, const char* file_content);

	void CreateEmptyFile(const char* file_name, lib_dir lib = NO_LIB, const char* extension = "");
	void DestroyFile(const char* file_name, lib_dir lib = NO_LIB, const char* extension = "");
	bool ExistisFile(const char* file_name, lib_dir lib = NO_LIB, const char* extension = "");
	bool copyFileTo(const char* full_path_src, lib_dir dest_lib = NO_LIB, const char* extension = "", std::string file_new_name = "");
	int getFileLastTimeMod(const char* file_name, lib_dir lib = NO_LIB, const char* extension = "");
	bool FindInDirectory(const char* directory, const char* file_name, std::string& final_path); // File name with extension
	std::string getPathFromLibDir(lib_dir r_type);


	void getFileNameFromPath(std::string& str);
	bool getPath(std::string & str);
	void getExtension(std::string& str);
	void FormFullPath(std::string& path, const char* file_name, lib_dir lib, const char* extension);
	bool removeExtension(std::string& str);
	bool removePath(std::string& str);





};

#endif


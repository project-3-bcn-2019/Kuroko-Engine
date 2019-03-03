#ifndef __MODULE_RESOURCES__
#define __MODULE_RESOURCES__

#include "Module.h"
#include "Resource.h"
#include "Timer.h"
#include "ModuleScene.h"
#include <map>
#include <list>

enum lib_dir;
enum PrimitiveTypes;
class ModuleResourcesManager: public Module
{
public:
	ModuleResourcesManager(Application* app, bool start_enabled = true);
	~ModuleResourcesManager();

	bool Init(const JSON_Object* config); // Set values from config
	bool Start();						  // Export everything on the assets folder if it has no metadata
	// Check assets library and test all the assets against their .meta files every 3 seconds
	// If:
	// 1. Metadata has no asset -> Erase metadata and file from library
	// 2. Asset has no metadata -> Generate metadata and import it to library
	// 3. Asset and metadata have different "last edited time" values -> Destroy old file in library, reimport file to library and change metadata
	update_status Update(float dt);		
	update_status PostUpdate(float dt);
	bool CleanUp();						  // Unload all the ocupied resources


	// To be executed allways when engine starts. 
	// Fills resources list for each file in assets, and the ones which don't have .meta are exported to library and given.meta
	void ManageUITextures();
	void GeneratePrimitiveResources();
	void GenerateLibraryAndMeta();
	bool ManageFile(std::string file_path, resource_deff& deff); // Retruns true if a new resource has to be generated
	resource_deff ManageMeta(std::string path, std::string name, std::string extension);
	void CleanMeshesFromLibrary(std::string prefab_binary);
	// Generates Imports file to library, with the name of the uuid
	// IMPORTANT: If the resource is an FBX, it exports all the meshes as different binarys, and only one for the FBX which is a scene,
	// with the transformations and hierarchy, and uuids of the meshes that it contains. Only one .metadata is generated, and it points
	// to that file.	
	resource_deff ManageAsset(std::string path, std::string name, std::string extension);
		
	// To be executed on start if in game mode
	void GenerateResources();
	void GenerateFromMapFile(JSON_Value* file, ResourceType type);

	// Regenerate resources if the file is modified (for the moment)
	void ManageAssetModification();

	void CompileAndGenerateScripts();

	Resource* getResource(uint uuid); // Returns the resource with the corresponding UUID, if not found returns nullptr
	Resource* getPrimitiveMeshResource(PrimitiveTypes primitive);
	int assignResource(uint uuid); // It adds 1 to the "components used by" of a certain resource
	int deasignResource(uint uuid); // It substracts 1 to the "components used by" of a certain resource
	// Creates a resource from a .meta that contains uuid, type, extension, timeCreated...
	// IMPORTANT: Resources for meshes will be created by the constructor of the scene resource, as it can access
	// binarys, original asset, uuid, and type, of course.
	Resource* newResource(resource_deff deff);		
	void LoadResource(uint uuid);						// Iterates resource list, looks for the resource and allows it to load


	// Looks for the file's metadata. If it doesn't find it, it calls ImportToLibrary(const char* file)
	// When it has it, looks for the resource, and tells it to LoadToMemory(). The only type of asset that
	// will generate objects in the scene is FBX, as it is a scene itself. The other ones will just be loaded to memory,
	// and showcased in the resources debug UI
	void Load3dObjectToScene(const char* file);
	uint getResourceUuid(const char* file); // Will not assign the resource 
	uint getResourceUuid(const char* name, ResourceType type);
	uint getMeshResourceUuid(const char* Parent3dObject, const char* name);
	uint getAnimationResourceUuid(const char* Parent3dObject, const char* name);
	uint getAnimationResourceUuid(const char* name);
	uint getBoneResourceUuid(const char* Parent3dObject, const char* name);
	uint getAudioResourceUuid(const char* name);

	void CleanResources() { cleanResources = true; }


	// Set
	void setRefreshRatio(int refresh_ratio) { update_ratio = refresh_ratio; }
	// Get
	void getMeshResourceList(std::list<resource_deff>& meshes);
	void getScriptResourceList(std::list<resource_deff>& scripts);
	void getAnimationResourceList(std::list<resource_deff>& animations);
	void getSceneResourceList(std::list<resource_deff>& scenes, std::list<resource_deff> ignore);
	void getAudioResourceList(std::list<resource_deff>& audio);
	std::string getPrefabPath(const char* prefab_name); // Retruns the binary file for the given prefab so it can be loaded
	std::string getScenePath(const char* scene_name); // Retruns the binary file for the given scene so it can be loaded
	// get texture list
	// get scene list
	// To put in UI remember
	// Helpers
	std::string uuid2string(uint uuid);			// Converts a uuid into a file name, to be able to read from the library
	const char* assetExtension2type(const char* extension); // Converts an extension into a resource file
	ResourceType type2enumType(const char* type);
	const char* enumType2binaryExtension(ResourceType type);
	lib_dir enumType2libDir(ResourceType type);

private:
	std::map<uint, Resource*> resources;	
	std::map<PrimitiveTypes, Resource*> primitive_resources;



	int update_ratio; // Library will update every second, not every frame
	Timer update_timer;

	void CleanMeta();
	void CleanLibrary();
	void ReloadVM();
	void ReleaseResourcesScriptHandles();
	bool reloadResources = false;
	bool cleanResources = false;
	bool reloadVM = false;
};

#endif
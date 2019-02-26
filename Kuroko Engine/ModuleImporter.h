#ifndef __MODULE_IMPORTER
#define __MODULE_IMPORTER
#include "Module.h"
#include "Material.h"
#include <vector>

class GameObject;
class ComponentMesh;
class aiNode;
class aiScene;
class AudioFile;
class Mesh;

//enum ImportType { I_NONE, I_GOBJ, I_TEXTURE, I_FX, I_MUSIC};

struct material_resource_deff {
	uint resource_uuid_diffuse = 0;
	uint resource_uuid_ambient = 0;
	uint resource_uuid_normals = 0;
	uint resource_uuid_lightmap = 0;
};

class ModuleImporter : public Module
{
public:
	ModuleImporter(Application* app, bool start_enabled = true);
	~ModuleImporter();

	bool Init(const JSON_Object* config);
	bool CleanUp();

	void* ImportTexturePointer(const char* file);


	bool ImportTexture(const char* file_original_name, std::string file_binary_name); // Retruns true on successful import and false if it wasn't
	bool ImportScene(const char* file_original_name, std::string file_binary_name);	  
	// A script binary is a JSON, containinf the script code in a string, and the name of the class, which will be the same that the script's name
	bool ImportScript(const char* file_original_name, std::string file_binary_name);

	//Own file format
	void ImportMeshToKR(const char* file, Mesh* mesh);
	void ImportTextureToDDS(const char* file);
	Mesh* ExportMeshFromKR(const char* file); 
	Texture* LoadTextureFromLibrary(const char* file);

	void ImportSounds();

private:
	void ImportNodeToSceneRecursive(const aiNode& node, const aiScene& scene, JSON_Value* json_scene, const std::vector<material_resource_deff>& in_mat_id, uint parent = 0);  // TODO: Add a parameter for mat id, a vector of JSON_Objects*
	void ImportMaterialsFromNode(const aiScene& scene, std::vector<material_resource_deff>& out_mat_id);

};
void logAssimp(const char* message, char* user);
#endif
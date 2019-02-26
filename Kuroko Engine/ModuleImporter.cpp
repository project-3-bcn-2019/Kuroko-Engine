#include "ModuleImporter.h"
#include "GameObject.h"
#include "Globals.h"
#include "ComponentMesh.h"
#include "Material.h"
#include "ModuleUI.h"
#include "ModuleScene.h"
#include "Application.h"
#include "Applog.h"
#include "ModuleAudio.h"
#include "Camera.h"
#include "FileSystem.h"
#include "ComponentTransform.h"
#include "Transform.h"
#include "Timer.h"
#include "Random.h"
#include "ModuleResourcesManager.h"

#include "glew-2.1.0\include\GL\glew.h"
#include "SDL\include\SDL_opengl.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include <fstream>
#include <string>


#include "Assimp\include\cimport.h"
#include "Assimp\include\scene.h"
#include "Assimp\include\postprocess.h"
#include "Assimp\include\cfileio.h"
#include "Assimp\include\Logger.hpp"

#include "DevIL/include/IL/il.h"
#include "DevIL/include/IL/ilu.h"
#include "DevIL/include/IL/ilut.h"

#pragma comment (lib, "Assimp/lib/assimp.lib")

#pragma comment (lib, "DevIL/lib/DevIL.lib")
#pragma comment (lib, "DevIL/lib/ILU.lib")
#pragma comment (lib, "DevIL/lib/ILUT.lib")

#include "ModuleCamera3D.h"

ModuleImporter::ModuleImporter(Application* app, bool start_enabled) : Module(app, start_enabled) {
	name = "importer";
}

ModuleImporter::~ModuleImporter() {}

bool ModuleImporter::Init(const JSON_Object* config)
{

	struct aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, "log.txt");
	stream.callback = logAssimp;
	aiAttachLogStream(&stream);

	ilutRenderer(ILUT_OPENGL);
	ilInit();
	iluInit();
	ilutInit();

	ilutRenderer(ILUT_OPENGL);

	ilEnable(IL_CONV_PAL);
	ilutEnable(ILUT_OPENGL_CONV);

	return true;
}

bool ModuleImporter::CleanUp()
{
	aiDetachAllLogStreams();
	return true;
}


void* ModuleImporter::ImportTexturePointer(const char* file) {
	std::string extension = file;
	App->fs.getExtension(extension);

	if (extension == ".bmp" || extension == ".dds" || extension == ".jpg" || extension == ".pcx" || extension == ".png"
		|| extension == ".raw" || extension == ".tga" || extension == ".tiff") {
		std::string texture_name = file;
		App->fs.getFileNameFromPath(texture_name);

		ILuint il_id = 0;
		ilGenImages(1, &il_id);
		ilBindImage(il_id);

		ilLoadImage(file);

		ILinfo ImageInfo;
		iluGetImageInfo(&ImageInfo);
		if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
			iluFlipImage();

		GLuint TexId = ilutGLBindTexImage();

		Texture* tex = new Texture(TexId, texture_name.c_str(), false);  // Never export to library textures used by the editor

		ilDeleteImage(il_id);

		app_log->AddLog("Success loading texture: %s", file);
		return tex;
	}

	app_log->AddLog("Error loading texture [incompatible format]: %s", file);

	return nullptr;
	/*
	//if (expected_filetype == I_NONE || expected_filetype == I_MUSIC)
	//{
	//	if (extension == ".mod" || extension == ".midi" || extension == ".mp3" || extension == ".flac")
	//	{
	//		std::string name = PathFindFileNameA(file);
	//		if (AudioFile* last_audio_file = App->audio->LoadAudio(file, name.c_str(), MUSIC))
	//		{
	//			app_log->AddLog("Success loading music: %s", file);
	//			return last_audio_file;
	//		}
	//		else
	//			app_log->AddLog("Error loading music: %s", file);
	//	}
	//}
	//if (expected_filetype == I_NONE || expected_filetype == I_FX)
	//{
	//	if (extension == ".wav" || extension == ".aiff" || extension == ".riff" || extension == ".ogg" || extension == ".voc")
	//	{
	//		std::string name = PathFindFileNameA(file);
	//		if (AudioFile* last_audio_file = App->audio->LoadAudio(file, name.c_str(), FX))
	//		{
	//			app_log->AddLog("Success loading fx: %s", file);
	//			return last_audio_file;
	//		}
	//		else
	//			app_log->AddLog("Error loading fx: %s", file);
	//	}
	//}*/
}

void ModuleImporter::ImportNodeToSceneRecursive(const aiNode & node, const aiScene & scene, JSON_Value * objects_array, const std::vector<material_resource_deff>& in_mat_id, uint parent)
{
	std::string name = node.mName.C_Str();
	if (name.find("$Assimp") != std::string::npos)
	{
		for (int i = 0; i < node.mNumChildren; i++)
			ImportNodeToSceneRecursive(*node.mChildren[i], scene, objects_array, in_mat_id, parent);

		return;
	}


	JSON_Value* game_object = json_value_init_object();
	JSON_Value* components = json_value_init_array();
	uint object_uuid = random32bits();

	json_object_set_string(json_object(game_object), "name", node.mName.C_Str());
	json_object_set_boolean(json_object(game_object), "static", false);
	json_object_set_number(json_object(game_object), "UUID", object_uuid);
	if (parent != 0)
		json_object_set_number(json_object(game_object), "Parent", parent);
	json_object_set_value(json_object(game_object), "Components", components);


	// Import and store transform
	aiVector3D pos = { 0.0f, 0.0f, 0.0f };
	aiVector3D scl = { 1.0f, 1.0f, 1.0f };;
	aiQuaternion rot;
	node.mTransformation.Decompose(scl, rot, pos);
	Transform* trans = new Transform();
	JSON_Value* transform_component = json_value_init_object();
	JSON_Value* local_transform = json_value_init_object();
	trans->Set(float3(pos.x, pos.y, pos.z), Quat(rot.x, rot.y, rot.z, rot.w), float3(scl.x, scl.y, scl.z));
	trans->Save(json_object(local_transform));
	json_object_set_string(json_object(transform_component), "type", "transform"); // Set type
	json_object_set_value(json_object(transform_component), "local", local_transform); //Set local transform
	json_array_append_value(json_array(components), transform_component);			// Add component to components
	delete trans;


	// Add aabb so it is reloaded 
	JSON_Value* aabb_component = json_value_init_object();
	json_object_set_string(json_object(aabb_component), "type", "AABB");
	json_array_append_value(json_array(components), aabb_component);

	std::vector<std::string> mesh_names;
	int same_name_separator = 1;
	for (int i = 0; i < node.mNumMeshes; i++) {
		// Import, store and delete mesh
		Mesh* mesh = new Mesh(*scene.mMeshes[node.mMeshes[i]], scene, node.mName.C_Str());
		JSON_Value* mesh_component = json_value_init_object();	
		uint uuid_number = random32bits();																// Create mesh component
		std::string uuid = std::to_string(uuid_number);
		std::string binary_full_path = MESHES_FOLDER + uuid + OWN_MESH_EXTENSION;
		json_object_set_string(json_object(mesh_component), "type", "mesh");			// Set type
		json_object_set_string(json_object(mesh_component), "mesh_binary_path", binary_full_path.c_str()); // Set mesh (used for deleting binary file when asset is deleted)
		json_object_set_number(json_object(mesh_component), "mesh_resource_uuid", uuid_number);				// Set uuid


		std::string mesh_name = node.mName.C_Str();

		// Look if a mesh with the same name was loaded previously
		for (auto it = mesh_names.begin(); it != mesh_names.end(); it++) {
			if ((*it) == mesh_name) { // If it was, add the separator, two meshes can't have the same name
				mesh_name = mesh_name + "_" + std::to_string(same_name_separator);
				same_name_separator++;
				break;
			}
		}
		mesh_names.push_back(mesh_name);

		json_object_set_string(json_object(mesh_component), "mesh_name", mesh_name.c_str());				// Mesh name(only for file readability)
		json_object_set_string(json_object(mesh_component), "primitive_type", "NONE");


		if (scene.mMeshes[node.mMeshes[i]]->mMaterialIndex < in_mat_id.size()) {// If it has a material
		material_resource_deff deff = in_mat_id.at(scene.mMeshes[node.mMeshes[i]]->mMaterialIndex);
		JSON_Value* material = json_value_init_object();
		json_object_set_number(json_object(material), "diffuse_resource_uuid", deff.resource_uuid_diffuse);
		//json_object_set_string(json_object(material), "ambient binary", deff.binary_path_ambient.c_str());
		//json_object_set_string(json_object(material), "normal binary", deff.binary_path_normal.c_str());
		//json_object_set_string(json_object(material), "lightmap binary", deff.binary_path_lightmap.c_str());
		json_object_set_value(json_object(mesh_component), "material", material);			// Add material to component mesh
		}

		json_array_append_value(json_array(components), mesh_component);			// Add component mesh to components
		ImportMeshToKR(uuid.c_str(), mesh);				// Import mesh
		delete mesh;									// TODO: Delete mesh

		app_log->AddLog("Imported mesh with %i vertices", scene.mMeshes[node.mMeshes[i]]->mNumVertices);
	}

	json_array_append_value(json_array(objects_array), game_object);    // Add gameobject to gameobject array


	for (int i = 0; i < node.mNumChildren; i++)
		ImportNodeToSceneRecursive(*node.mChildren[i], scene, objects_array, in_mat_id, object_uuid);

}

void ModuleImporter::ImportMaterialsFromNode(const aiScene & scene, std::vector<material_resource_deff>& out_mat_id)
{
	aiString path;
	for (int i = 0; i < scene.mNumMaterials; i++)
	{
		material_resource_deff material_deff;
		if (scene.mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE))
		{
			scene.mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &path);
			// Find the path in assets (or not), ManageAsset, then add the uuid to the material
			std::string file_name = path.C_Str();
			std::string final_path;
			App->fs.removePath(file_name);
			if (App->fs.FindInDirectory(ASSETS_FOLDER, file_name.c_str(), final_path)) {
				std::string path, name, extension;
				path = name = extension = final_path;
				App->fs.getPath(path);
				App->fs.getFileNameFromPath(name);
				App->fs.getExtension(extension);
				resource_deff managed_res = App->resources->ManageAsset(path, name, extension); 
				material_deff.resource_uuid_diffuse = managed_res.uuid;
			}
			else
				app_log->AddLog("%s texture could not be found", file_name.c_str());
		}
		if (scene.mMaterials[i]->GetTextureCount(aiTextureType_AMBIENT))
		{
			path.Clear();
			scene.mMaterials[i]->GetTexture(aiTextureType_AMBIENT, 0, &path);
			// Find the path in library (or not), ManageAsset, then add the binary to the material

		}
		if (scene.mMaterials[i]->GetTextureCount(aiTextureType_NORMALS))
		{
			path.Clear();
			scene.mMaterials[i]->GetTexture(aiTextureType_NORMALS, 0, &path);
			// Find the path in library (or not), ManageAsset, then add the binary to the material

		}
		if (scene.mMaterials[i]->GetTextureCount(aiTextureType_LIGHTMAP))
		{
			path.Clear();
			scene.mMaterials[i]->GetTexture(aiTextureType_LIGHTMAP, 0, &path);
			// Find the path in library (or not), ManageAsset, then add the binary to the material

		}
		out_mat_id.push_back(material_deff);
	}
}

bool ModuleImporter::ImportScene(const char * file_original_name, std::string file_binary_name) {

	const aiScene* imported_scene = aiImportFile(file_original_name, aiProcessPreset_TargetRealtime_MaxQuality);

	if (imported_scene) {
		JSON_Value* scene = json_value_init_object();
		JSON_Value* objects_array = json_value_init_array();	// Array of objects in the scene
		//std::vector<uint> mat_id;
		//LoadMaterials(*imported_scene, mat_id);

		json_object_set_value(json_object(scene), "Game Objects", objects_array); // Add array to file

		std::vector<material_resource_deff> out_mat_deff;
		ImportMaterialsFromNode(*imported_scene, out_mat_deff);
		ImportNodeToSceneRecursive(*imported_scene->mRootNode, *imported_scene, objects_array, out_mat_deff);

		std::string path;
		App->fs.FormFullPath(path, file_binary_name.c_str(), LIBRARY_3DOBJECTS, JSON_EXTENSION);
		json_serialize_to_file(scene, path.c_str());
		json_value_free(scene);
		aiReleaseImport(imported_scene);
		app_log->AddLog("Success importing file %s (scene)", file_original_name);

		return true;
	}
	else
		app_log->AddLog("Error importing file %s (scene): imported scene was null", file_original_name);

	return false;
}
void logAssimp(const char* message, char* user) {
	app_log->AddLog("%s", message);
}

bool ModuleImporter::ImportTexture(const char * file_original_name, std::string file_binary_name) {

	std::string path, name, extension;
	path = name = extension = file_original_name;
	App->fs.getExtension(extension);
	App->fs.getPath(path);
	App->fs.getFileNameFromPath(name);
	bool is_dds = extension == ".dds";

	ILuint il_id = 0;
	ilGenImages(1, &il_id);
	ilBindImage(il_id);

	ilLoadImage(file_original_name);

	GLuint TexId = ilutGLBindTexImage();

	ILinfo ImageInfo;
	iluGetImageInfo(&ImageInfo);
	if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
		iluFlipImage();

	Texture* tex = new Texture(TexId, file_binary_name.c_str(), !is_dds);  // If it is a dds, don't compress it

	ilDeleteImage(il_id);

	if (is_dds) { 									// We copy and paste file in library folder.
		if (!App->fs.copyFileTo(file_original_name, LIBRARY_TEXTURES, DDS_EXTENSION, file_binary_name))
			app_log->AddLog("%s could not be copied to Library/Textures", file_original_name);
	}


	delete tex;

	app_log->AddLog("Success importing texture: %s", file_original_name);
	return true;
}

bool ModuleImporter::ImportScript(const char* file_original_name, std::string file_binary_name) {
	JSON_Value* script_json = json_value_init_object();

	std::string file_content = App->fs.GetFileString(file_original_name);
	std::string class_name = file_original_name;
	App->fs.getFileNameFromPath(class_name);



	json_object_set_string(json_object(script_json), "code", file_content.c_str());
	json_object_set_string(json_object(script_json), "class_name", class_name.c_str());

	std::string path;
	App->fs.FormFullPath(path, file_binary_name.c_str(), LIBRARY_SCRIPTS, JSON_EXTENSION);

	json_serialize_to_file(script_json, path.c_str());
	json_value_free(script_json);

	return true;
}

void ModuleImporter::ImportMeshToKR(const char * file, Mesh* mesh) {

	uint header[5];
	uint vert_num, poly_count = 0;
	bool has_normals, has_colors, has_texcoords = false;

	// Get the data
	mesh->getData(vert_num, poly_count, has_normals, has_colors, has_texcoords);

	header[0] = vert_num;
	header[1] = poly_count;
	header[2] = has_normals;
	header[3] = has_colors;
	header[4] = has_texcoords;

	// Knowing the size of the file, we can create the buffer in which it all will be stored
	uint size = sizeof(header);			// header
	size += sizeof(float3);				// centroid
	size += sizeof(float3)*vert_num;	// vertices
	size += sizeof(Tri)*poly_count;		// tris

	if(has_normals)		size += sizeof(float3)*vert_num;	// normals
	if(has_colors)		size += sizeof(float3)*vert_num;	// colors
	if(has_texcoords)	size += sizeof(float2)*vert_num;	// texcoords

	char* data = new char[size];
	char* cursor = data;

	// First we store the header
	uint bytes = sizeof(header);
	memcpy(cursor, header, bytes);
	cursor += bytes;

	// Centroid
	bytes = sizeof(float3);
	memcpy(cursor, &mesh->getCentroid(), bytes);
	cursor += bytes;

	// Vertices
	const float3* vertices = mesh->getVertices();
	bytes = sizeof(float3)*vert_num;
	memcpy(cursor, vertices, bytes);
	cursor += bytes;

	// Tris
	const Tri* tris = mesh->getTris();
	bytes = sizeof(Tri)*poly_count;
	memcpy(cursor, tris, bytes);
	cursor += bytes;

	// Normlas
	if (has_normals)
	{
		const float3* normals = mesh->getNormals();
		bytes = sizeof(float3)*vert_num;
		memcpy(cursor, normals, bytes);
		cursor += bytes;
	}

	// Colors
	if (has_colors)
	{
		const float3* colors = mesh->getColors();
		bytes = sizeof(float3)*vert_num;
		memcpy(cursor, colors, bytes);
		cursor += bytes;
	}

	// Tex coords
	if (has_texcoords)
	{
		const float2* tex_coords = mesh->getTexCoords();
		bytes = sizeof(float2)*vert_num;
		memcpy(cursor, tex_coords, bytes);
	}

	std::string filename = file;
	App->fs.getFileNameFromPath(filename);
	App->fs.ExportBuffer(data, size, filename.c_str(), LIBRARY_MESHES, OWN_MESH_EXTENSION);
	app_log->AddLog("Saved %s as own file format", filename.c_str());

	delete data;
}

void ModuleImporter::ImportTextureToDDS(const char* texture_name) {
	ILuint size;
	char *data;
	ilSetInteger(IL_DXTC_FORMAT, IL_DXT5);// To pick a specific DXT compression use
	size = ilSaveL(IL_DDS, NULL, 0); // Get the size of the data buffer
	if (size > 0) {
		data = new char[size]; // allocate data buffer
		if (ilSaveL(IL_DDS, data, size) > 0) // Save to buffer with the ilSaveIL function
			App->fs.ExportBuffer(data, size, texture_name, LIBRARY_TEXTURES, DDS_EXTENSION);
		delete data; 
	}
}

Mesh * ModuleImporter::ExportMeshFromKR(const char * file)
{
	uint num_vertices		= 0;
	uint num_tris			= 0;
	bool import_normals		= false;
	bool import_colors		= false;
	bool import_tex_coords	= false;

	float3* vertices	= nullptr;
	Tri* tris			= nullptr;
	float3* normals		= nullptr;
	float3* colors		= nullptr;
	float2* tex_coords	= nullptr;
	float3 centroid		= float3::zero;

	// Import buffer from file
	if (!App->fs.ExistisFile(file)) {
		app_log->AddLog("Couldn't load mesh, not found in library");
		return nullptr;
	}

	char* buffer = App->fs.ImportFile(file);
	char* cursor = buffer;

	// Read the header
	uint header[5];
	uint bytes = sizeof(header);
	memcpy(header, cursor, bytes);
	cursor += bytes;

	num_vertices = header[0];
	num_tris = header[1];
	import_normals = header[2];
	import_colors = header[3];
	import_tex_coords = header[4];

	bytes = sizeof(float3);
	memcpy(&centroid, cursor, bytes);
	cursor += bytes;

	// Read vertices
	bytes = sizeof(float3)*num_vertices;
	vertices = new float3[num_vertices];
	memcpy(vertices, cursor, bytes);
	cursor += bytes;

	// Read tris
	bytes = sizeof(Tri)*num_tris;
	tris = new Tri[num_tris];
	memcpy(tris, cursor, bytes);
	cursor += bytes;

	// Read normals (if any)
	if (import_normals) 
	{
		bytes = sizeof(float3)*num_vertices;
		normals = new float3[num_vertices];
		memcpy(normals, cursor, bytes);
		cursor += bytes;
	}

	// Read colors (if any)
	if (import_colors) 
	{
		bytes = sizeof(float3)*num_vertices;
		colors = new float3[num_vertices];
		memcpy(colors, cursor, bytes);
		cursor += bytes;
	}

	// Read tex_coords (if any)
	if (import_tex_coords) 
	{
		bytes = sizeof(float2)*num_vertices;
		tex_coords = new float2[num_vertices];
		memcpy(tex_coords, cursor, bytes);
	}

	delete buffer;
	app_log->AddLog("Loaded mesh %s from own file format", file);
	return new Mesh(vertices,tris,normals,colors,tex_coords, num_vertices, num_tris, centroid);
}

Texture * ModuleImporter::LoadTextureFromLibrary(const char * file) {
	return new Texture(ilutGLLoadImage((char*)file), file, false);
}

void ModuleImporter::ImportSounds()
{
	char folderPath[256];
	GetCurrentDirectory(256, folderPath);
	// Add the assets folder to the path
	std::string path = folderPath;
	path += "\\Assets\\Sounds/*";

	WIN32_FIND_DATA file;
	HANDLE search_handle = FindFirstFile(path.c_str(), &file);
	if (search_handle)
	{
		do
		{
			if (file.dwFileAttributes && FILE_ATTRIBUTE_DIRECTORY)
			{
				// Check type of flie
				std::string name = file.cFileName;
				std::string extension = "";
				for (int i = name.size() - 1; i >= 0; i--)
					if (name[i] == '.' | name[i] == ' ')
						break;
					else
						extension = name[i] + extension;

				if (extension == "bnk")
				{
					std::string source = path;
					source.pop_back();
					source += name;

					std::string destiny = folderPath;
					destiny += "\\Library\\Sounds/" + name;

					std::ifstream  src(source, std::ios::binary);
					std::ofstream  dst(destiny, std::ios::binary);

					dst << src.rdbuf();
				}
			}
		} while (FindNextFile(search_handle, &file));
		FindClose(search_handle);
	}
}
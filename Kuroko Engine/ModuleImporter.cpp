#include "ModuleImporter.h"
#include "GameObject.h"
#include "Globals.h"
#include "ComponentMesh.h"
#include "Material.h"
#include "ModuleImGUI.h"
#include "Application.h"
#include "Applog.h"

#include "glew-2.1.0\include\GL\glew.h"
#include "SDL\include\SDL_opengl.h"
#include <gl/GL.h>
#include <gl/GLU.h>

#include "Assimp\include\cimport.h"
#include "Assimp\include\scene.h"
#include "Assimp\include\postprocess.h"
#include "Assimp\include\cfileio.h"

#include "DevIL/include/IL/il.h"
#include "DevIL/include/IL/ilu.h"
#include "DevIL/include/IL/ilut.h"

#pragma comment (lib, "Assimp/lib/assimp.lib")

#pragma comment (lib, "DevIL/lib/DevIL.lib")
#pragma comment (lib, "DevIL/lib/ILU.lib")
#pragma comment (lib, "DevIL/lib/ILUT.lib")


ModuleImporter::ModuleImporter(Application* app, bool start_enabled) : Module(app, start_enabled) {
	name = "importer";
}

ModuleImporter::~ModuleImporter() {}

bool ModuleImporter::Init(JSON_Object* config)
{
	struct aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
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

GameObject* ModuleImporter::LoadFBX(const char* file)
{
	GameObject* root_obj = nullptr;
	const aiScene* imported_scene = aiImportFile(file, aiProcessPreset_TargetRealtime_MaxQuality);

	if (imported_scene)
	{
		root_obj = LoadAssimpNode(imported_scene->mRootNode, imported_scene);
		aiReleaseImport(imported_scene);
	}
	else
		app_log->AddLog("Error loading scene %s", file);

	return root_obj;
}

GameObject* ModuleImporter::LoadAssimpNode(aiNode* node, const aiScene* scene, GameObject* parent)
{
	GameObject* root_obj = new GameObject(node->mName.C_Str(), parent);

	for (int i = 0; i < node->mNumMeshes; i++)
	{
		root_obj->addComponent(new ComponentMesh(root_obj, scene->mMeshes[node->mMeshes[i]]));
		app_log->AddLog("New mesh with %d vertices", scene->mMeshes[node->mMeshes[i]]->mNumVertices);
	}

	for (int i = 0; i < node->mNumChildren; i++)
		root_obj->addChild(LoadAssimpNode(node->mChildren[i], scene, root_obj));

	return root_obj;
}

bool ModuleImporter::LoadRootMesh(const char* file, ComponentMesh* component_to_load)
{
	const aiScene* imported_scene = aiImportFile(file, aiProcessPreset_TargetRealtime_MaxQuality);

	if (imported_scene && imported_scene->HasMeshes())
	{
		component_to_load->ClearData();
		if (component_to_load->loaded = component_to_load->LoadFromAssimpMesh(imported_scene->mMeshes[0]))
			component_to_load->LoadDataToVRAM();
		aiReleaseImport(imported_scene);
		return true;
	}
	else
		app_log->AddLog("Error loading scene %s", file);

	return false;
}

Material* ModuleImporter::LoadTex(char* file, Mat_Wrap wrap, Mat_MinMagFilter min_filter, Mat_MinMagFilter mag_filter)
{
	Material* mat = new Material(ilutGLLoadImage(file));
	mat->setParameters(wrap, min_filter, mag_filter);
	return mat;
}


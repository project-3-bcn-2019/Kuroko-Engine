#include "Application.h"
#include "Globals.h"
#include "Applog.h"
#include "ModuleShaders.h"
#include "ResourceShader.h"
#include "FileSystem.h"


ResourceShader::ResourceShader(resource_deff deff) : Resource(deff)
{
}


ResourceShader::~ResourceShader()
{
}

void ResourceShader::LoadToMemory() 
{
	shaderObject = LoadShader();

	if (shaderObject)
	{
		if (App->shaders->CompileShader(shaderObject))
			loaded_in_memory = true;
		else
			UnloadFromMemory();
	}
	else
	{
		app_log->AddLog("Error loading binary %s", binary.c_str());
	}
}

void ResourceShader::UnloadFromMemory() 
{
	loaded_in_memory = false;
	RELEASE(shaderObject);
}

void ResourceShader::Reload()
{
	shaderObject->script = App->fs.ImportFile(binary.c_str());
	App->shaders->CompileShader(shaderObject);
	App->shaders->RecompileAllPrograms();
}

bool ResourceShader::SaveShader()
{
	//TODO if we create a new resourceShader we have to take into
	//acount that asset would not exist
	bool ret = false;

	if (shaderObject)
	{
		int size = strlen(shaderObject->script.c_str());
		++size;
		char* buffer = new char[size];
		strcpy(buffer, shaderObject->script.c_str());

		App->fs.ExportBuffer(buffer, size, asset.c_str());
		ret = true;
	}

	return ret;
}

Shader* ResourceShader::LoadShader()
{
	Shader* ret = new Shader();

	if (App->fs.ExistisFile(binary.c_str()))
	{
		ret->name = asset;
		ret->script = App->fs.ImportFile(binary.c_str());

		std::string extension = binary.c_str();
		App->fs.getExtension(extension);

		if (extension == VERTEXSHADER_EXTENSION)
		{
			ret->type = VERTEX;
		}
		else
		{
			ret->type = FRAGMENT;
		}
	}

	return ret;
}


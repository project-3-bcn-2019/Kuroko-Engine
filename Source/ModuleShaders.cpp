#include "Application.h"
#include "ModuleShaders.h"
#include "ModuleRenderer3D.h"
#include "ModuleWindow.h"
#include "Globals.h"
#include "Applog.h"
#include "FileSystem.h"

#include "glew-2.1.0/include/GL/glew.h"
#include "SDL\include\SDL_opengl.h"
#include <gl/GL.h>
#include <gl/GLU.h>

#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment( lib, "glew-2.1.0/lib/glew32.lib")
#pragma comment( lib, "glew-2.1.0/lib/glew32s.lib")


ModuleShaders::ModuleShaders(Application* app, bool start_enabled):Module(app,start_enabled)
{
	name = "Shaders";
}

ModuleShaders::~ModuleShaders()
{
	RELEASE(defVertexShader);
	RELEASE(defFragmentShader);
	RELEASE(defShaderProgram);
}

bool ModuleShaders::Init(const JSON_Object * config)
{
	bool ret = true;

	defVertexShader = new Shader(VERTEX);
	defFragmentShader = new Shader(FRAGMENT);
	defShaderProgram = new ShaderProgram();

	animationShader = new Shader(VERTEX);
	animationShaderProgram = new ShaderProgram();


	return ret;
}

bool ModuleShaders::InitializeDefaulShaders()
{
	//Default shaders
	bool findFile = App->fs.ExistisFile(defaultVertexFile.c_str(), LIBRARY_MATERIALS, ".vex");
	if (findFile)
	{
		defVertexShader->script = App->fs.ImportFile(defaultVertexFile.c_str());
	}
	else
	{
		CreateDefVertexShader();
	}

	CompileShader(defVertexShader);
	defShaderProgram->shaders.push_back(defVertexShader->shaderId);

	findFile = App->fs.ExistisFile(defaultFragmentFile.c_str(), LIBRARY_MATERIALS, ".frag");
	if (findFile)
	{
		defVertexShader->script = App->fs.ImportFile(defaultVertexFile.c_str());
	}
	else
	{
		CreateDefFragmentShader();
	}

	CompileShader(defFragmentShader);
	defShaderProgram->shaders.push_back(defFragmentShader->shaderId);

	CompileProgram(defShaderProgram);

	//Animation Shaders
	/*CompileShader(animationShader);
	animationShaderProgram->shaders.push_back(animationShader->shaderId);
	*/
	shaders.push_back(defVertexShader);
	shaders.push_back(defFragmentShader);
	//shaders.push_back(animationShader);

	shader_programs.push_back(defShaderProgram);
	/*shader_programs.push_back(animationShaderProgram);*/



	return true;
}

void ModuleShaders::CreateDefVertexShader()
{
	defVertexShader->script =
	"#version 330\n"

	"layout(location = 0) in vec3 position;\n"
	"layout(location = 1) in vec4 color;\n"
	"layout(location = 2) in vec2 texCoord;\n"
	"layout(location = 3) in vec3 normal;\n"

	"out vec4 ourColor;\n"
	"out vec2 TexCoord;\n"
	"out vec3 ret_normal;\n"
	"out vec3 FragPos;\n"

	"uniform mat4 model_matrix;\n"
	"uniform mat4 view;\n"
	"uniform mat4 projection;\n"

	"void main()\n"
	"{\n"
		"FragPos=vec3(model_matrix*vec4(position,1.0));\n"
		"gl_Position = projection * view * model_matrix * vec4(position,1.0f);\n"
		"ourColor = color;\n"
		"TexCoord = texCoord;\n"
		"ret_normal = normal;\n"
	"}\n";

	defVertexShader->name = "Defaul Vertex Shader";

	animationShader->script =
	"#version 330\n"

	"layout(location = 0) in vec3 Position;\n"
	"layout(location = 1) in vec2 TexCoord;\n"
	"layout(location = 2) in vec3 Normal;\n"
	"layout(location = 3) in ivec4 BoneIDs;\n"
	"layout(location = 4) in vec4 Weights;\n"

	"out vec2 TexCoord0;\n"
	"out vec3 Normal0;\n"
	"out vec3 WorldPos0;\n"

	"const int MAX_BONES = 100;\n"

	"uniform mat4 view;\n"
	"uniform mat4 projection;\n"
	"uniform mat4 gBones[MAX_BONES];\n"

	"void main()\n"
	"{\n"
	"	mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0];\n"
	"	BoneTransform += gBones[BoneIDs[1]] * Weights[1];\n"
	"	BoneTransform += gBones[BoneIDs[2]] * Weights[2];\n"
	"	BoneTransform += gBones[BoneIDs[3]] * Weights[3];\n"

	"	vec4 PosL = BoneTransform * vec4(Position, 1.0);\n"
	"	gl_Position = projection * PosL;\n"
	"	TexCoord0 = TexCoord;\n"
	"	vec4 NormalL = BoneTransform * vec4(Normal, 0.0);\n"
	"	Normal0 = (view * NormalL).xyz;\n"
	"	WorldPos0 = (view * PosL).xyz;\n"
	"}\n";

	animationShader->name = "Defaul Animation Vertex Shader";
}

void ModuleShaders::CreateDefFragmentShader()
{
	defFragmentShader->script =
	"#version 330\n"

	"in vec3 FragPos;\n"
	"in vec4 ourColor;\n"
	"in vec2 TexCoord;\n"
	"in vec3 ret_normal;\n"

	"out vec4 color;\n"

	"uniform sampler2D ourTexture;\n"
	"uniform int test;\n"
	"uniform vec3 lightPos;\n"
	"uniform vec3 lightColor;\n"

	"void main()\n"
	"{\n"
		"if(test==1)\n"
		"{\n"
			"color=texture(ourTexture,TexCoord);\n"
		"}\n"

		"else \n"
		"{\n"
			//ambient
			"float ambientStrength=1.0;\n"
			"vec3 ambient= ambientStrength*lightColor;\n"

			//diffuse
			"vec3 norm = normalize(ret_normal);\n"
			"vec3 lightDir = normalize(lightPos - FragPos);\n"
			"float diff = max(dot(norm, lightDir), 0.0);\n"
			"vec3 diffuse = diff * lightColor;\n"

			"vec3 result = (ambient + diffuse) * vec3(ourColor.x,ourColor.y,ourColor.z);\n"
			"color=vec4(result, 1.0);\n"
		"}\n"
	"}\n";

	defFragmentShader->name = "Defaul Fragment Shader";

}

bool ModuleShaders::CompileShader(Shader* shader)
{
	bool ret = false;

	if (shader->shaderId != 0)
	{
		glDeleteShader(shader->shaderId);
	}

	if (shader->type == VERTEX)
	{
		shader->shaderId = glCreateShader(GL_VERTEX_SHADER);
	}
	else
	{
		shader->shaderId = glCreateShader(GL_FRAGMENT_SHADER);
	}
	
	int size = strlen(shader->script.c_str());
	++size;
	char* buffer = new char[size];
	strcpy(buffer, shader->script.c_str());

	glShaderSource(shader->shaderId, 1, &buffer, NULL);
	glCompileShader(shader->shaderId);

	int success;
	char Error[512];
	glGetShaderiv(shader->shaderId, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(shader->shaderId, 512, NULL, Error);
		app_log->AddLog(Error);
		ret = false;
	}
	else
	{
		if(shader->type == VERTEX)
			app_log->AddLog("Vertex Shader compiled successfully!");
		else
			app_log->AddLog("Fragment Shader compiled successfully!");

		ret = true;
	}

	return ret;
}

bool ModuleShaders::CompileProgram(ShaderProgram* program)
{
	bool ret = false;

	program->programID = glCreateProgram();

	for (int i = 0; i < program->shaders.size(); i++)
	{
		glAttachShader(program->programID, program->shaders[i]);
	}

	glLinkProgram(program->programID);

	for (int i = 0; i < program->shaders.size(); i++)
	{
		glDetachShader(program->programID, program->shaders[i]);
	}

	int success;
	char Error[512];
	glGetProgramiv(program->programID, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(program->programID, 512, NULL, Error);
		app_log->AddLog(Error);
		ret = false;
	}
	else
	{
		app_log->AddLog("Program linked and compiled Successfully!");
		ret = true;
	}

	return ret;
}

bool ModuleShaders::RecompileAllPrograms()
{
	for (int i = 0; i < shader_programs.size(); ++i)
	{
		for (int j = 0; j < shader_programs[i]->shaders.size(); ++j)
		{
			if(shader_programs[i]->shaders[j]== defFragmentShader->shaderId)
				CompileShader(defFragmentShader);
		}
		CompileProgram(shader_programs[i]);
	}
	return false;
}

bool ModuleShaders::IsProgramValid(uint program)
{
	bool ret = false;

	GLint success = 0;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
	if (success == GL_FALSE)
	{
		app_log->AddLog("Shader Program is not valid. ERROR: %s");
		ret = false;
	}
	else
	{
		app_log->AddLog("Shader Program is valid");
		app_log->AddLog("%d", program);
		ret = true;
	}

	return ret;
}

ShaderProgram * ModuleShaders::GetDefaultShaderProgram() const
{
	return defShaderProgram;
}

ShaderProgram * ModuleShaders::GetAnimationShaderProgram() const
{
	return animationShaderProgram;
}

Uniform::Uniform(std::string _name, std::string _type, int size)
{
	name = _name;
	data = new float[size];
	type = GetTypefromChar(_type);
	stringType = _type;
}

UniformType Uniform::GetTypefromChar(std::string _type)
{
	UniformType ret= U_INT;

	if (_type == "int")
		ret = U_INT;
	if (_type == "bool")
		ret = U_BOOL;
	if (_type == "float")
		ret = U_FLOAT;
	if (_type == "vec2")
		ret = U_VEC2;
	if (_type == "vec3")
		ret = U_VEC3;
	if (_type == "vec4")
		ret = U_VEC4;
	if (_type == "mat2")
		ret = U_MAT2;
	if (_type == "mat3")
		ret = U_MAT3;
	if (_type == "mat4")
		ret = U_MAT4;
	
	return ret;
}

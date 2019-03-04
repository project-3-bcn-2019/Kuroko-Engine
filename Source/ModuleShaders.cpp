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
	animationFragmentShader = new Shader(FRAGMENT);
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
	CompileShader(animationShader);
	CompileShader(animationFragmentShader);
	animationShaderProgram->shaders.push_back(animationShader->shaderId);
	animationShaderProgram->shaders.push_back(animationFragmentShader->shaderId);
	
	CompileProgram(animationShaderProgram);

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

	animationShader->script =
	"#version 330\n"

	"layout(location = 0) in vec3 position;\n"
	"layout(location = 1) in vec4 color;\n"
	"layout(location = 2) in vec2 texCoord;\n"
	"layout(location = 3) in vec3 normal;\n"
	"layout(location = 4) in ivec4 BoneIDs;\n"
	"layout(location = 5) in vec4 Weights;\n"
	"layout(location = 6) in int wCounter;\n"


	"out vec2 TexCoord;\n"
	"out vec3 ret_normal;\n"
	"out vec3 FragPos;\n"
	"out vec4 ourColor;\n"

	"const int MAX_BONES = 100;\n"
	
	"uniform mat4 model_matrix;\n"
	"uniform mat4 view;\n"
	"uniform mat4 projection;\n"
	"uniform mat4 gBones[MAX_BONES];\n"

	"void main()\n"
	"{\n"
		"vec4 totalLocalPos = vec4(0.0);\n"
		"vec4 totalNormal = vec4(0.0);\n"

		"for(int i=0;i<4;i++)\n"
		"{\n"
			"if(Weights[i]!=0)\n"
			"{\n"
				"mat4 boneTransform = gBones[BoneIDs[i]];\n"
				"vec4 posePosition = boneTransform * vec4(position,1.0);\n"
				"totalLocalPos += posePosition*Weights[i];\n"

				"vec4 worldNormal = boneTransform * vec4(normal,0.0);\n"
				"totalNormal += worldNormal * Weights[i];\n"
			"}\n"
		"}\n"

		"gl_Position = projection * view * vec4(totalLocalPos.xyz,1.0);\n"
		"ret_normal = totalNormal.xyz;\n"
		"TexCoord = texCoord;\n"
		"ourColor = color;\n"
	"}\n";
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


	animationFragmentShader->script =
	"#version 330\n"

	"in vec4 ourColor;\n"
	"out vec4 color;\n"

	"void main()\n"
	"{\n"
		"color = ourColor;\n"
	"}\n";
}

void ModuleShaders::CreateShaderFromTextEditor(char* script, ShaderType type)
{
	if (script!=nullptr)
	{
		Shader* aux_shader = new Shader(type);
		aux_shader->script = script;

		if (CompileShader(aux_shader))
		{
			all_Shaders.push_back(aux_shader);
		}
		else
		{
			app_log->AddLog("Unable to compile the shader!");
			RELEASE(aux_shader);
		}

	}
	else
	{
		app_log->AddLog("Shader without content!");
	}
}

bool ModuleShaders::CompileShader(Shader* shader)
{
	bool ret = false;

	if (shader->type == VERTEX)
	{
		shader->shaderId = glCreateShader(GL_VERTEX_SHADER);
	}
	else
	{
		shader->shaderId = glCreateShader(GL_FRAGMENT_SHADER);
	}
	
	glShaderSource(shader->shaderId, 1, &shader->script, NULL);
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

ShaderProgram * ModuleShaders::GetDefaultShaderProgram() const
{
	return defShaderProgram;
}

ShaderProgram * ModuleShaders::GetAnimationShaderProgram() const
{
	return animationShaderProgram;
}



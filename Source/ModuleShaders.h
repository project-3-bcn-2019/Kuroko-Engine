#ifndef __MODULE_SHADERS__
#define __MODULE_SHADERS__

#include "Module.h"

enum ShaderType
{
	VERTEX,
	FRAGMENT
};

enum UniformType
{
	U_INT, U_BOOL, U_FLOAT, U_VEC2, U_VEC3, U_VEC4, U_MAT2, U_MAT3, U_MAT4
};

struct Uniform
{
	Uniform(std::string _name, std::string _type, int size);

	UniformType GetTypefromChar(std::string _type);

	UniformType type;
	std::string name;
	std::string stringType;
	float* data;

};

struct Shader
{
	Shader(ShaderType type) :type(type) {};

	std::string name = "";
	std::string script = "";
	ShaderType type;
	uint shaderId=0;

	std::vector<Uniform*> uniforms;
};

struct ShaderProgram
{
	uint programID = 0;
	std::vector<uint> shaders;
};


class ModuleShaders : public Module
{
public:
	
	ModuleShaders(Application* app, bool start_enabled = true);
	~ModuleShaders();


	bool Init(const JSON_Object* config);

	bool InitializeDefaulShaders();

	void CreateDefVertexShader();
	void CreateDefFragmentShader();

	bool CompileShader(Shader* shader);
	bool CompileProgram(ShaderProgram* program);
	bool RecompileAllPrograms();

	ShaderProgram* GetDefaultShaderProgram()const;
	ShaderProgram* GetAnimationShaderProgram()const;

public:

	std::vector<Shader*> shaders;
	std::vector<ShaderProgram*> shader_programs;

private:

	std::string defaultVertexFile = "DefaultVertexShader.vex";
	std::string defaultFragmentFile = "DefaultPixelShader.fmt";

	Shader* defVertexShader=nullptr;
	Shader* defFragmentShader=nullptr;
	Shader* animationShader = nullptr;
	
	ShaderProgram* defShaderProgram=nullptr;
	ShaderProgram* animationShaderProgram = nullptr;

	
};

#endif // !__MODULE_SHADERS__

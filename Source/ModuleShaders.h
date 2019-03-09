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

};

struct Shader
{
	Shader(ShaderType type) :type(type) {};

	std::string name = "";
	char* script = nullptr;
	ShaderType type;
	uint shaderId=0;
};

struct ShaderProgram
{
	uint programID = 0;
	std::vector<uint> shaders;
};

struct Uniform 
{
	Uniform(std::string _name, UniformType _type, int size)
	{
		name = _name;
		type = _type;
		data = new float[size];
	}

	UniformType type;
	std::string name;
	float* data;
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

	void CompileShader(Shader* shader);
	void CompileProgram(ShaderProgram* program);

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

#ifndef __MODULE_SHADERS__
#define __MODULE_SHADERS__

#include "Module.h"

enum ShaderType
{
	VERTEX,
	FRAGMENT
};

struct Shader
{
	Shader(ShaderType type) :type(type) {};

	char* script = nullptr;
	ShaderType type;
	uint shaderId=0;
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

	void CompileShader(Shader* shader);
	void CompileProgram(ShaderProgram* program);

	ShaderProgram* GetDefaultShaderProgram()const;
	ShaderProgram* GetAnimationShaderProgram()const;

private:

	std::string defaultVertexFile = "DefaultVertexShader.vex";
	std::string defaultFragmentFile = "DefaultPixelShader.fmt";

	Shader* defVertexShader=nullptr;
	Shader* defFragmentShader=nullptr;
	Shader* animationShader = nullptr;
	Shader* animationFragmentShader = nullptr;
	
	ShaderProgram* defShaderProgram=nullptr;
	ShaderProgram* animationShaderProgram = nullptr;
	
};

#endif // !__MODULE_SHADERS__

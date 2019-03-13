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
	std::string name;
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
	void CreateShaderFromTextEditor(char* script,ShaderType type);

	bool CompileShader(Shader* shader);
	bool CompileProgram(ShaderProgram* program);

	bool CompileAnimationProgram();

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

	std::list<Shader*> all_Shaders;
	
};

#endif // !__MODULE_SHADERS__

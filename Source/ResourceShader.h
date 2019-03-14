#ifndef __RESOURCESHADER_H__
#define __RESOURCESHADER_H__

#include "Resource.h"

enum ShaderType;
struct Shader;

class ResourceShader : public Resource
{
public:

	ResourceShader(resource_deff deff);
	~ResourceShader();

	void LoadToMemory();
	void UnloadFromMemory();

	bool SaveShader();
	Shader* LoadShader();

public:

	Shader* shaderObject = nullptr;

};

#endif // !__RESOURCESHADER_H__

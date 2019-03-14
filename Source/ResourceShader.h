#ifndef __RESOURCESHADER_H__
#define __RESOURCESHADER_H__

#include "Resource.h"

enum ShaderType;

class ResourceShader : public Resource
{
public:

	ResourceShader(resource_deff deff);
	~ResourceShader();

public:

	std::string name = "";
	std::string script = "";
	//ShaderType type;
	uint shaderId = 0;

};

#endif // !__RESOURCESHADER_H__

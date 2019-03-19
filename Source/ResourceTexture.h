#ifndef __TEXTURE_RESOURCE
#define __TEXTURE_RESOURCE
#include "Resource.h"

struct Texture;
class ResourceTexture :
	public Resource {
public:
	ResourceTexture(resource_deff deff);
	~ResourceTexture();
	void LoadToMemory();
	void UnloadFromMemory();
public:
	Texture* texture = nullptr;
	bool drawn_in_UI = false;
};

#endif

#ifndef __3DOBJECT_RESOURCE
#define __3DOBJECT_RESOURCE
#include "Resource.h"
class Resource3dObject :
	public Resource {
public:
	Resource3dObject(resource_deff deff);
	~Resource3dObject();
	void LoadToMemory();
	void UnloadFromMemory();
};
#endif

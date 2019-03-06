#ifndef _COMPONENT_ANIMATOR
#define _COMPONENT_ANIMATOR

#include "Component.h"

class ComponentAnimator :
	public Component
{
public:
	ComponentAnimator(GameObject* gameobject) : Component(gameobject, ANIMATOR) {};
	ComponentAnimator(JSON_Object* deff, GameObject* parent);
	~ComponentAnimator();

	uint getAnimationGraphResource() const { return graph_resource_uuid; }
	void setAnimationGraphResource(uint uuid) { graph_resource_uuid = uuid; }

	void Save(JSON_Object* config);

private:

	uint graph_resource_uuid;
};

#endif // !_COMPONENT_ANIMATOR
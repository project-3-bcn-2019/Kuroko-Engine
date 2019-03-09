#ifndef _COMPONENT_ANIMATOR
#define _COMPONENT_ANIMATOR

#include "Component.h"
#include <map>

enum variableType;

class ComponentAnimator :
	public Component
{
public:
	ComponentAnimator(GameObject* gameobject) : Component(gameobject, ANIMATOR) {};
	ComponentAnimator(JSON_Object* deff, GameObject* parent);
	~ComponentAnimator();

	uint getAnimationGraphResource() const { return graph_resource_uuid; }
	void setAnimationGraphResource(uint uuid);

	void setInt(uint uuid, int value);
	int* getInt(uint uuid);
	void setFloat(uint uuid, float value);
	float* getFloat(uint uuid);
	void setString(uint uuid, const char* value);
	std::string* getString(uint uuid);
	void setBool(uint uuid, bool value);
	bool* getBool(uint uuid);

	void removeValue(variableType type, uint uuid);

	void Save(JSON_Object* config);

private:

	uint graph_resource_uuid;
	std::map<uint, int> ints;
	std::map<uint, float> floats;
	std::map<uint, std::string> strings;
	std::map<uint, bool> bools;
};

#endif // !_COMPONENT_ANIMATOR
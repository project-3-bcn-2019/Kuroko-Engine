#ifndef _COMPONENT_SCRIPT
#define _COMPONENT_SCRIPT
#include "Component.h"
#include "ResourceScript.h"
#include "ScriptData.h"
#include <vector>

class ComponentScript : public Component 
{
public:
	ComponentScript(GameObject* g_obj, uint resource_uuid = 0);
	ComponentScript(JSON_Object* deff, GameObject* parent);
	~ComponentScript();

	uint getResourceUUID() { return script_resource_uuid; }; 
	void assignScriptResource(uint resource_uuid);
	void LoadResource();
	void CleanUp();
	void DrawInspector(int id = 0);
	void Save(JSON_Object* config);

	ScriptData* instance_data = nullptr;
	std::string script_name;
private:

	uint script_resource_uuid = 0;
	void LinkScriptToObject();
};


#endif
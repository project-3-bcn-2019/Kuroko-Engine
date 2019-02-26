#ifndef _MODULE
#define _MODULE

class Application;
struct PhysBody3D;
#include "Globals.h"
#include "Parson\parson.h"
#include <string>

class Module
{
private :
	bool enabled;

public:

	Module(Application* parent, bool start_enabled = true)
	{}

	virtual ~Module()
	{}

	virtual bool Init(const JSON_Object* config) 
	{
		return true; 
	}

	virtual bool Start()
	{
		return true;
	}

	virtual update_status PreUpdate(float dt)
	{
		return UPDATE_CONTINUE;
	}

	virtual update_status Update(float dt)
	{
		return UPDATE_CONTINUE;
	}

	virtual update_status PostUpdate(float dt)
	{
		return UPDATE_CONTINUE;
	}

	virtual bool CleanUp() 
	{ 
		return true; 
	}

	virtual void SaveConfig(JSON_Object* config) const {}
	virtual void LoadConfig(const JSON_Object* config) {}

	std::string name;
};
#endif
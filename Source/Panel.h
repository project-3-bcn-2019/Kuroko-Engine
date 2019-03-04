#ifndef __PANEL_H__
#define __PANEL_H__

#include <string>
#include "ImGui/imgui.h"

class Panel
{
public:
	Panel(const char* name, bool _active = false) : name(name), active (_active) {}
	virtual ~Panel() {}

	virtual void Draw() {}
	void toggleActive() { active = !active; }

	bool isActive() const { return active; }
	const char* getName() const { return name.c_str(); }

protected:

	std::string name = "";
	bool active = false;

};


#endif // !__PANEL_H__
#pragma once

#include <string>
#include "ImGui/imgui.h"

class Panel
{
public:
	Panel(const char* name) : name(name) {}
	virtual ~Panel() {}

	virtual void Draw() {}
	void toggleActive() { active = !active; }

	bool isActive() const { return active; }
	const char* getName() const { return name.c_str(); }

protected:

	std::string name = "";
	bool active = false;

};
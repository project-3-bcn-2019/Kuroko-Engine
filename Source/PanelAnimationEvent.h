#pragma once

#include "Panel.h"
#include <list>

class Component;
class ComponentAnimation;
class ResourceAnimation;
class GameObject;

class PanelAnimationEvent : public Panel
{
public:

	PanelAnimationEvent(const char* name);

	bool fillInfo();

	~PanelAnimationEvent();

	void Draw();
	
};
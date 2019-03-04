#ifndef __PANELINSPECTOR_H__
#define __PANELINSPECTOR_H__

#include "Panel.h"
#include <list>

class GameObject;

class PanelObjectInspector : public Panel
{
public:

	PanelObjectInspector(const char* name, bool active);
	~PanelObjectInspector();

	void Draw();

	void DrawTagSelection(GameObject * object);

public:
	//bool disable_keyboard_control = false;
};

#endif
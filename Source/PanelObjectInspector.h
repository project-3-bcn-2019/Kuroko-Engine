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

private:
	void DrawTagSelection(GameObject * object);
};

#endif
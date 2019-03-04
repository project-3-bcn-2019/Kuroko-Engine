#ifndef __PANELHIERARCHY_H__
#define __PANELHIERARCHY_H__

#include "Panel.h"
#include <list>

class GameObject;

class PanelHierarchyTab : public Panel
{
public:

	PanelHierarchyTab(const char* name, bool active);
	~PanelHierarchyTab();

	void Draw();
	bool DrawHierarchyNode(GameObject & game_object, int & id);

public:
	//bool disable_keyboard_control = false;
};

#endif
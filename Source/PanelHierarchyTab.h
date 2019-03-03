#ifndef __PANELHIERARCHY_H__
#define __PANELHIERARCHY_H__

#include "Panel.h"
#include <list>

class PanelHierarchyTab : public Panel
{
public:

	PanelHierarchyTab(const char* name);
	~PanelHierarchyTab();

	void Draw();
};

#endif
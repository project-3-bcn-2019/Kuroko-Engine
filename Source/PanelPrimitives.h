#ifndef __PANELPRIMITIVES_H__
#define __PANELPRIMITIVES_H__

#include "Panel.h"
#include <list>

class GameObject;

class PanelPrimitives : public Panel
{
public:

	PanelPrimitives(const char* name, bool active);
	~PanelPrimitives();

	void Draw();

private:

};

#endif
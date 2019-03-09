#ifndef __PANELTIMECONTROL_H__
#define __PANELTIMECONTROL_H__

#include "Panel.h"
#include <list>

class GameObject;

class PanelTimeControl : public Panel
{
public:

	PanelTimeControl(const char* name, bool active);
	~PanelTimeControl();

	void Draw();

private:


};

#endif
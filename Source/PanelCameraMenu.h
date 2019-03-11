#ifndef __PANELCAMERA_H__
#define __PANELCAMERA_H__

#include "Panel.h"
#include <list>


class PanelCameraMenu : public Panel
{
public:

	PanelCameraMenu(const char* name);
	~PanelCameraMenu();

	void Draw();

private:


};

#endif
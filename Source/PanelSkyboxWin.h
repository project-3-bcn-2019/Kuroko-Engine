#ifndef __PANELSKYBOX_H__
#define __PANELSKYBOX_H__

#include "Panel.h"
#include <list>

class GameObject;

class PanelSkyboxWin : public Panel
{
public:

	PanelSkyboxWin(const char* name);
	~PanelSkyboxWin();

	void Draw();

private:

};

#endif

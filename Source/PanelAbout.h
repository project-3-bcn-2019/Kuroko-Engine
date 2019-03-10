#ifndef __PANELABOUT_H__
#define __PANELABOUT_H__

#include "Panel.h"
#include <list>


class PanelAbout : public Panel
{
public:

	PanelAbout(const char* name);
	~PanelAbout();

	void Draw();

private:


};

#endif
#ifndef __PANELVIEWPORTS_H__
#define __PANELVIEWPORTS_H__

#include "Panel.h"
#include <list>


class PanelViewports : public Panel
{
public:

	PanelViewports(const char* name);
	~PanelViewports();

	void Draw();

private:


};

#endif
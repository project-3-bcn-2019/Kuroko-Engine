#ifndef __PANELQUADTREECONFIG_H__
#define __PANELQUADTREECONFIG_H__

#include "Panel.h"
#include <list>


class PanelQuadtreeConfig : public Panel
{
public:

	PanelQuadtreeConfig(const char* name);
	~PanelQuadtreeConfig();

	void Draw();

private:


};

#endif
#ifndef __PANELCONFIGURATION_H__
#define __PANELCONFIGURATION_H__

#include "Panel.h"
#include <list>


class PanelConfiguration : public Panel
{
public:

	PanelConfiguration(const char* name, bool active);
	~PanelConfiguration();

	void Draw();	

private:
	void DrawGraphicsLeaf() const;
	void DrawWindowConfigLeaf() const;
	void DrawHardwareLeaf() const;
	void DrawApplicationLeaf() const;
	void DrawEditorPreferencesLeaf() const;
	void DrawResourcesLeaf() const;
};

#endif
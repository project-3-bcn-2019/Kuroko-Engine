#ifndef __PANELASSETSWINDOW_H__
#define __PANELASSETSWINDOW_H__

#include "Panel.h"
#include "FileSystem.h"
#include <list>

class GameObject;

class PanelAssetsWin : public Panel
{
public:

	PanelAssetsWin(const char* name, bool active);
	~PanelAssetsWin();

	void Draw();

private:
	void DrawAssetInspector();

private:
	std::string asset_window_path = ASSETS_FOLDER;
	std::string selected_asset;
};

#endif
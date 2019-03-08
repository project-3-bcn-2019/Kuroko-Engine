#ifndef __PANELSHADER_H__
#define __PANELSHADER_H__

#include "Panel.h"

#include <string>

class TextEditor;

class PanelShader :
	public Panel
{
public:
	
	PanelShader(const char* name, bool _active = false);
	~PanelShader();

	void Draw();

private:

	TextEditor shader_editor;		//USED IN SHADER EDITOR
	std::string open_shader_path;	//USED IN SHADER EDITOR

};

#endif // !__PANELSHADER_H__

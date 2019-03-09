#ifndef __PANELSHADER_H__
#define __PANELSHADER_H__

#include "Panel.h"

#include <string>

class TextEditor;
struct Shader;
struct Uniform;

class PanelShader :
	public Panel
{
public:
	
	PanelShader(const char* name, bool _active = false);
	~PanelShader();

	void Draw();
	void SaveShader(Shader* shader);

private:

	TextEditor shader_editor;		//USED IN SHADER EDITOR
	std::string open_shader_path;	//USED IN SHADER EDITOR

	Shader* current_shader = nullptr;
	
	std::vector<Uniform*> shader_uniforms;

};

#endif // !__PANELSHADER_H__

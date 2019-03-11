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
	void AddUniform();

private:

	TextEditor shader_editor;		//USED IN SHADER EDITOR
	std::string open_shader_path;	//USED IN SHADER EDITOR

	Shader* current_shader = nullptr;
	
	std::vector<Uniform*> shader_uniforms;
	
	bool vertex=false;
	bool fragment=false;

	char uniform_name[256] = "";
	char* selected_type = nullptr;

	std::vector<char*> uniform_types= {"int","bool","float","vec2","vec3","vec4","mat2","mat3","mat4"};



	int uniform_size = 1;

};

#endif // !__PANELSHADER_H__

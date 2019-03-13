#ifndef _COMPONENTBUTTONUI_
#define _COMPONENTBUTTONUI_

#include "Component.h"
#include <list>

class ComponentRectTransform;
class ComponentImageUI;
class ResourceTexture;

enum ButtonState {
	B_IDLE = 0,
	B_MOUSEOVER,
	B_PRESSED
};

struct WrenCall {

	WrenCall(std::string script, std::string method): script_name(script), method_name(method) {}
	std::string script_name;
	std::string method_name;
};

class ComponentButtonUI :public Component
{
public:
	ComponentButtonUI(GameObject* parent);
	ComponentButtonUI(JSON_Object * deff, GameObject * parent);
	~ComponentButtonUI();

	bool Update(float dt)override;
	void DrawInspector(int id = 0) override;
	void WhenPressed();
	void Save(JSON_Object* config) override;

	const ResourceTexture* getResourceTexture(ButtonState state);
	void setResourceTexture(ResourceTexture* tex, ButtonState state); 
	void DeassignTexture(ButtonState state);
	inline void setState(ButtonState _state) { state = _state; ChangeGOImage(); };// for debug, may be obsolete
	ButtonState getState() { return state; };
	void ChangeGOImage();

	std::list<WrenCall> callbacks;

	ButtonState state = B_IDLE;

	inline void doFadeIn() {	alpha = 0.0f; fadingIn = true;}
	inline void doFadeOut() { alpha = 1.0f; fadingOut = true; }
	
	
private:

	ComponentRectTransform * rectTransform = nullptr;
	ComponentImageUI* image = nullptr;

	ResourceTexture* idle = nullptr;
	ResourceTexture* hover = nullptr;
	ResourceTexture* pressed = nullptr;

	void FadeIn();
	void FadeOut();
	bool fadingIn = false;
	bool fadingOut = false;
	float alpha = 1.0f;
};

#endif
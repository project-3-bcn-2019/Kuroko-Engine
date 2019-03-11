#ifndef _COMPONENTBUTTONUI_
#define _COMPONENTBUTTONUI_

#include "Component.h"


class ComponentRectTransform;
class ComponentImageUI;
class ResourceTexture;

enum ButtonState {
	B_IDLE = 0,
	B_MOUSEOVER,
	B_PRESSED
};

//struct ButtonLink {
//	ComponentButtonUI up = nullptr;
//	ComponentButtonUI down = nullptr;
//	ComponentButtonUI left = nullptr;
//	ComponentButtonUI right = nullptr;
//};

class ComponentButtonUI :public Component
{
public:
	ComponentButtonUI(GameObject* parent);
	ComponentButtonUI(JSON_Object * deff, GameObject * parent);
	~ComponentButtonUI();

	bool Update(float dt)override;
	void ChangeFocus();
	void Save(JSON_Object* config) override;

	const ResourceTexture* getResourceTexture(ButtonState state);
	void setResourceTexture(ResourceTexture* tex, ButtonState state); 
	void DeassignTexture(ButtonState state);
	inline void setState(ButtonState _state) { state = _state; ChangeGOImage(); };// for debug, may be obsolete
	ButtonState getState() { return state; };
	void ChangeGOImage();

	inline void doFadeIn() {	alpha = 0.0f; fadingIn = true;}
	inline void doFadeOut() { alpha = 1.0f; fadingOut = true; }

	//bool isFocus() const { return focus; }
	//void setFocus(bool _focus) { focus = _focus; }

public:
	ButtonState state = B_IDLE;
	//ButtonLink button_link;
	
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
	//bool focus = false;
	float alpha = 1.0f;

	ComponentButtonUI* up = nullptr;
	ComponentButtonUI* down = nullptr;
	ComponentButtonUI* left = nullptr;
	ComponentButtonUI* right = nullptr;
};

#endif
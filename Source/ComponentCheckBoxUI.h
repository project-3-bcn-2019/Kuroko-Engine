#ifndef _COMPONENTCHECKBOXUI_
#define _COMPONENTCHECKBOXUI_

#include "Component.h"

class ComponentRectTransform;
class ComponentImageUI;
class ResourceTexture;

enum CheckBoxState {
	CH_IDLE = 0,
	CH_PRESSED
};

class ComponentCheckBoxUI :
	public Component
{
public:
	ComponentCheckBoxUI(GameObject* parent);
	ComponentCheckBoxUI(JSON_Object* deff, GameObject* parent);
	~ComponentCheckBoxUI();

	bool Update(float dt)override;

	void Save(JSON_Object* config) override;

	inline const ResourceTexture* getResourceTexture(CheckBoxState state) { return state == CH_IDLE ? idle : pressed; }
	inline void setResourceTexture(ResourceTexture* tex, CheckBoxState state) { state == CH_IDLE ? idle = tex : pressed = tex; ChangeGOImage(); }
	inline void DeassignTexture(CheckBoxState state) { state == CH_IDLE ? idle = nullptr : pressed = nullptr; ChangeGOImage(); }

	inline void Press() { state == CH_IDLE ? state = CH_PRESSED : state = CH_IDLE; ChangeGOImage(); } // for debug, may be obsolete
	inline const bool isPressed() { return state == CH_PRESSED; }// for debug, may be obsolete

	CheckBoxState state = CH_IDLE;

private:
	void ChangeGOImage();
	bool isMouseOver();
	void CheckState();

	ComponentRectTransform * rectTransform = nullptr;
	ComponentImageUI* image = nullptr;

	ResourceTexture* idle = nullptr;
	ResourceTexture* pressed = nullptr;
};

#endif
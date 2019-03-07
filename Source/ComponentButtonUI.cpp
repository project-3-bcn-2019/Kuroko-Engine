#include "ComponentButtonUI.h"
#include "ComponentImageUI.h"
#include "ComponentRectTransform.h"
#include "GameObject.h"


ComponentButtonUI::ComponentButtonUI(GameObject* parent) : Component(parent, UI_BUTTON)
{
	rectTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);
	image = (ComponentImageUI*)parent->getComponent(UI_IMAGE);
}

ComponentButtonUI::ComponentButtonUI(JSON_Object * deff, GameObject * parent) : Component(parent, UI_BUTTON)
{
	rectTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);
	image = (ComponentImageUI*)parent->getComponent(UI_IMAGE);
}

ComponentButtonUI::~ComponentButtonUI()
{
	rectTransform = nullptr;
	image = nullptr;
	pressed = nullptr;
	idle = nullptr;
	hover = nullptr;
}

bool ComponentButtonUI::Update(float dt)
{

	if (fadingOut) {
		FadeOut();
	}
	if (fadingIn) {
		FadeIn();
	}

	return true;
}



const ResourceTexture * ComponentButtonUI::getResourceTexture(ButtonState state)
{
	switch (state)
	{
	case B_IDLE:
		return idle;
	case B_MOUSEOVER:
		return hover;
	case B_PRESSED:
		return pressed;
	}
}

void ComponentButtonUI::setResourceTexture(ResourceTexture * tex, ButtonState state)
{
	switch (state)
	{
	case B_IDLE:
		idle = tex;
		break;
	case B_MOUSEOVER:
		hover = tex;
		break;
	case B_PRESSED:
		pressed = tex;
		break;
	}
	ChangeGOImage();
}

void ComponentButtonUI::DeassignTexture(ButtonState state)
{
	switch (state)
	{
	case B_IDLE:
		idle = nullptr;
		break;
	case B_MOUSEOVER:
		hover = nullptr;
		break;
	case B_PRESSED:
		pressed = nullptr;
		break;
	}
	ChangeGOImage();
}

void ComponentButtonUI::ChangeGOImage()
{
	switch (state)
	{
	case B_IDLE:
		image->setResourceTexture(idle);
		break;
	case B_MOUSEOVER:
		image->setResourceTexture(hover); 
		break;
	case B_PRESSED:
		image->setResourceTexture(pressed);
		break;
	}
}

void ComponentButtonUI::Save(JSON_Object * config)
{
}


void ComponentButtonUI::FadeIn()
{
	alpha += DELTA_ALPHA;
	if (alpha >= 1.0f) {
		fadingIn = false;
		alpha = 1.0f;
	}

	image->doFadeIn();
}

void ComponentButtonUI::FadeOut()
{
	alpha -= DELTA_ALPHA;
	if (alpha <= 0.0f) {
		fadingOut = false;
		alpha = 0.0f;
	}
	image->doFadeOut();
}
#include "ComponentButtonUI.h"
#include "Application.h"
#include "ComponentImageUI.h"
#include "ComponentRectTransform.h"
#include "GameObject.h"
#include "ResourceTexture.h"
#include "ModuleResourcesManager.h"

ComponentButtonUI::ComponentButtonUI(GameObject* parent) : Component(parent, UI_BUTTON)
{
	rectTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);
	image = (ComponentImageUI*)parent->getComponent(UI_IMAGE);
}

ComponentButtonUI::ComponentButtonUI(JSON_Object * deff, GameObject * parent) : Component(parent, UI_BUTTON)
{
	rectTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);
	image = (ComponentImageUI*)parent->getComponent(UI_IMAGE);

	alpha = json_object_get_number(deff, "alpha");
	state = ButtonState((int)json_object_get_number(deff, "state"));

	const char* texPath = json_object_dotget_string(deff, "textureIdle");
	if (texPath && strcmp(texPath, "missing reference") != 0) {
		uint uuid = App->resources->getResourceUuid(texPath);
		idle = (ResourceTexture*)App->resources->getResource(uuid);
	}
	texPath = json_object_dotget_string(deff, "textureHover");
	if (texPath && strcmp(texPath, "missing reference") != 0) {
		uint uuid = App->resources->getResourceUuid(texPath);
		hover = (ResourceTexture*)App->resources->getResource(uuid);
	}
	texPath = json_object_dotget_string(deff, "texturePressed");
	if (texPath && strcmp(texPath, "missing reference") != 0) {
		uint uuid = App->resources->getResourceUuid(texPath);
		pressed = (ResourceTexture*)App->resources->getResource(uuid);
	}
	ChangeGOImage();
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
	json_object_set_string(config, "type", "UIbutton");
	json_object_set_number(config, "alpha", alpha);
	json_object_set_number(config, "state", state);

	std::string texName = std::string("missing_reference");
	if (idle) {  //If it has a texture
		texName = idle->asset;
	}
	json_object_dotset_string(config, "textureIdle", texName.c_str());
	texName = "missing_reference";

	if (hover) {  
		texName = hover->asset;
	}
	json_object_dotset_string(config, "textureHover", texName.c_str());
	texName = "missing_reference";

	if (pressed) {
		texName = pressed->asset;
	}
	json_object_dotset_string(config, "texturePressed", texName.c_str());

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
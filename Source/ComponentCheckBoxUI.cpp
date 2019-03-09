#include "ComponentCheckBoxUI.h"
#include "Application.h"
#include "ComponentImageUI.h"
#include "ComponentRectTransform.h"
#include "GameObject.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleResourcesManager.h"
#include "ResourceTexture.h"

ComponentCheckBoxUI::ComponentCheckBoxUI(GameObject* parent) : Component(parent, UI_CHECKBOX)
{
	rectTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);
	image = (ComponentImageUI*) parent->getComponent(UI_IMAGE);
}

ComponentCheckBoxUI::ComponentCheckBoxUI(JSON_Object * deff, GameObject * parent) : Component(parent, UI_CHECKBOX)
{
	rectTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);
	image = (ComponentImageUI*)parent->getComponent(UI_IMAGE);

	
	state = CheckBoxState((int)json_object_get_number(deff, "state"));

	const char* texPath = json_object_dotget_string(deff, "textureIdle");
	if (texPath && strcmp(texPath, "missing reference") != 0) {
		uint uuid = App->resources->getResourceUuid(texPath);
		idle = (ResourceTexture*)App->resources->getResource(uuid);
	}
	
	texPath = json_object_dotget_string(deff, "texturePressed");
	if (texPath && strcmp(texPath, "missing reference") != 0) {
		uint uuid = App->resources->getResourceUuid(texPath);
		pressed = (ResourceTexture*)App->resources->getResource(uuid);
	}
	ChangeGOImage();
}


ComponentCheckBoxUI::~ComponentCheckBoxUI()
{
	rectTransform = nullptr;
	image = nullptr;
	pressed = nullptr;
	idle = nullptr;
}

bool ComponentCheckBoxUI::Update(float dt)
{
	CheckState();
	return true;
}

void ComponentCheckBoxUI::Save(JSON_Object * config)
{
	json_object_set_string(config, "type", "UIcheckbox"); 
	json_object_set_number(config, "state", state);

	std::string texName = std::string("missing_reference");
	if (idle) {  //If it has a texture
		texName = idle->asset;
	}
	json_object_dotset_string(config, "textureIdle", texName.c_str());
	
	texName = "missing_reference";

	if (pressed) {
		texName = pressed->asset;
	}
	json_object_dotset_string(config, "texturePressed", texName.c_str());
}


void ComponentCheckBoxUI::ChangeGOImage()
{
	switch (state)
	{
	case CH_IDLE:
		image->setResourceTexture(idle);
		break;
	case CH_PRESSED:
		image->setResourceTexture(pressed);
		break;	
	}
}

bool ComponentCheckBoxUI::isMouseOver()
{
	//TO CHECK (PRESSED INSIDE CANVAS)
	float x = (((App->input->GetMouseX() / (float)App->window->main_window->width) * 2) - 1);
	float y = (((((float)App->window->main_window->height - (float)App->input->GetMouseY()) / (float)App->window->main_window->height) * 2) - 1);
		
	if (rectTransform->getGlobalPos().x <= x &&x <= rectTransform->getGlobalPos().x + rectTransform->getWidth() &&
		rectTransform->getGlobalPos().y <= y && y <= rectTransform->getGlobalPos().y + rectTransform->getHeight()) {
		return true;
	}
	else
		return false;
}

void ComponentCheckBoxUI::CheckState() {
	
	if (isMouseOver()) {
		if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN && state == CH_PRESSED) {
			state = CH_IDLE;
			if (idle != nullptr)
				ChangeGOImage();
		}
		else if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN && state != CH_PRESSED) {
			state = CH_PRESSED;
			if (pressed != nullptr)
				ChangeGOImage();
		}
	}
}
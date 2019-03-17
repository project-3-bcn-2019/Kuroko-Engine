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

#include "ModuleUI.h"
#include "Material.h"

std::string openFileWID(bool isfile = false);

ComponentCheckBoxUI::ComponentCheckBoxUI(GameObject* parent) : Component(parent, UI_CHECKBOX)
{
	rectTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);
	image = (ComponentImageUI*) parent->getComponent(UI_IMAGE);

	image->setInspectorDraw(false);
}

ComponentCheckBoxUI::ComponentCheckBoxUI(JSON_Object * deff, GameObject * parent) : Component(parent, UI_CHECKBOX)
{
	rectTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);
	image = (ComponentImageUI*)parent->getComponent(UI_IMAGE);

	
	state = CheckBoxState((int)json_object_get_number(deff, "state"));

	const char* texPath = json_object_dotget_string(deff, "textureIdle");
	if (texPath && strcmp(texPath, "missing reference") != 0) {
		uint texUUID = 0;
		if (!App->is_game || App->debug_game)
			texUUID = App->resources->getResourceUuid(texPath);
		else
		{
			std::string texName = texPath;
			App->fs.getFileNameFromPath(texName);
			texUUID = App->resources->getTextureResourceUuid(texName.c_str());
		}
		App->resources->assignResource(texUUID);
		idle = (ResourceTexture*)App->resources->getResource(texUUID);
	}
	
	texPath = json_object_dotget_string(deff, "texturePressed");
	if (texPath && strcmp(texPath, "missing reference") != 0) {
		uint texUUID = 0;
		if (!App->is_game || App->debug_game)
			texUUID = App->resources->getResourceUuid(texPath);
		else
		{
			std::string texName = texPath;
			App->fs.getFileNameFromPath(texName);
			texUUID = App->resources->getTextureResourceUuid(texName.c_str());
		}
		App->resources->assignResource(texUUID);
		pressed = (ResourceTexture*)App->resources->getResource(texUUID);
	}
	ChangeGOImage();

	image->setInspectorDraw(false);
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

bool ComponentCheckBoxUI::DrawInspector(int id)
{
	if (ImGui::CollapsingHeader("UI CheckBox"))
	{		
		
		int w = 0; int h = 0;
		if (getResourceTexture(CH_IDLE) != nullptr) {
			getResourceTexture(CH_IDLE)->texture->getSize(w, h);
		}

		ImGui::Text("Idle texture data: \n x: %d\n y: %d", w, h); 
		ImGui::Image(getResourceTexture(CH_IDLE) != nullptr ? (void*)getResourceTexture(CH_IDLE)->texture->getGLid() : (void*)App->gui->ui_textures[NO_TEXTURE]->getGLid(), ImVec2(128, 128));

		if (ImGui::Button("Load(from asset folder)##Dif: Load"))
		{
			std::string texture_path = openFileWID();
			uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
			if (new_resource != 0) {
				App->resources->assignResource(new_resource);
				if (getResourceTexture(CH_IDLE) != nullptr)
					App->resources->deasignResource(getResourceTexture(CH_IDLE)->uuid);
				setResourceTexture((ResourceTexture*)App->resources->getResource(new_resource), CH_IDLE);
			}
		}
		ImGui::Separator();


		int w2 = 0; int h2 = 0;
		if (getResourceTexture(CH_PRESSED) != nullptr) {
			getResourceTexture(CH_PRESSED)->texture->getSize(w2, h2);
		}

		ImGui::Text("Pressed texture data: \n x: %d\n y: %d", w2, h2);
		ImGui::Image(getResourceTexture(CH_PRESSED) != nullptr ? (void*)getResourceTexture(CH_PRESSED)->texture->getGLid() : (void*)App->gui->ui_textures[NO_TEXTURE]->getGLid(), ImVec2(128, 128));

		if (ImGui::Button("Load(from asset folder)##Dif: Load2"))
		{
			std::string texture_path = openFileWID();
			uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
			if (new_resource != 0) {
				App->resources->assignResource(new_resource);
				if (getResourceTexture(CH_PRESSED) != nullptr)
					App->resources->deasignResource(getResourceTexture(CH_PRESSED)->uuid);
				setResourceTexture((ResourceTexture*)App->resources->getResource(new_resource), CH_PRESSED);
			}
		}

		ImGui::Separator();
		// For debug
		bool pressed = isPressed();
		ImGui::Checkbox("Pressed", &pressed);
		if (pressed != isPressed()) { Press(); }
	}

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
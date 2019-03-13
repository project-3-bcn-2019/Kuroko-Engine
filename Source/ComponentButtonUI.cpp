#include "ComponentButtonUI.h"
#include "Application.h"
#include "ComponentImageUI.h"
#include "ComponentRectTransform.h"
#include "GameObject.h"
#include "ResourceTexture.h"
#include "ModuleResourcesManager.h"
#include "ModuleUI.h"
#include "Material.h"

#include "ImGui/imgui.h"

std::string openFileWID(bool isfile = false);

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

void ComponentButtonUI::DrawInspector(int id)
{
	if (ImGui::CollapsingHeader("UI Button"))
	{
		ButtonState state;//debug
		ImGui::Image(getResourceTexture(B_IDLE) != nullptr ? (void*)getResourceTexture(B_IDLE)->texture->getGLid() : (void*)App->gui->ui_textures[NO_TEXTURE]->getGLid(), ImVec2(128, 128));
		ImGui::SameLine();

		int w = 0; int h = 0;
		if (getResourceTexture(B_IDLE) != nullptr) {
			getResourceTexture(B_IDLE)->texture->getSize(w, h);
		}

		ImGui::Text("Idle texture data: \n x: %d\n y: %d", w, h);

		if (ImGui::Button("Load(from asset folder)##Dif: Load"))
		{
			std::string texture_path = openFileWID();
			uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
			if (new_resource != 0) {
				App->resources->assignResource(new_resource);
				if (getResourceTexture(B_IDLE) != nullptr)
					App->resources->deasignResource(getResourceTexture(B_IDLE)->uuid);
				setResourceTexture((ResourceTexture*)App->resources->getResource(new_resource), B_IDLE);
			}
		}

		ImGui::Image(getResourceTexture(B_MOUSEOVER) != nullptr ? (void*)getResourceTexture(B_MOUSEOVER)->texture->getGLid() : (void*)App->gui->ui_textures[NO_TEXTURE]->getGLid(), ImVec2(128, 128));
		ImGui::SameLine();

		int w3 = 0; int h3 = 0;
		if (getResourceTexture(B_MOUSEOVER) != nullptr) {
			getResourceTexture(B_MOUSEOVER)->texture->getSize(w3, h3);
		}

		ImGui::Text("Hover texture data: \n x: %d\n y: %d", w3, h3);

		if (ImGui::Button("Load(from asset folder)##Dif: Load2"))
		{
			std::string texture_path = openFileWID();
			uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
			if (new_resource != 0) {
				App->resources->assignResource(new_resource);
				if (getResourceTexture(B_MOUSEOVER) != nullptr)
					App->resources->deasignResource(getResourceTexture(B_MOUSEOVER)->uuid);
				setResourceTexture((ResourceTexture*)App->resources->getResource(new_resource), B_MOUSEOVER);
			}
		}


		ImGui::Image(getResourceTexture(B_PRESSED) != nullptr ? (void*)getResourceTexture(B_PRESSED)->texture->getGLid() : (void*)App->gui->ui_textures[NO_TEXTURE]->getGLid(), ImVec2(128, 128));
		ImGui::SameLine();

		int w2 = 0; int h2 = 0;
		if (getResourceTexture(B_PRESSED) != nullptr) {
			getResourceTexture(B_PRESSED)->texture->getSize(w2, h2);
		}

		ImGui::Text("Pressed texture data: \n x: %d\n y: %d", w2, h2);

		if (ImGui::Button("Load(from asset folder)##Dif: Load3"))
		{
			std::string texture_path = openFileWID();
			uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
			if (new_resource != 0) {
				App->resources->assignResource(new_resource);
				if (getResourceTexture(B_PRESSED) != nullptr)
					App->resources->deasignResource(getResourceTexture(B_PRESSED)->uuid);
				setResourceTexture((ResourceTexture*)App->resources->getResource(new_resource), B_PRESSED);
			}
		}
		// For debug
		bool idle = false;
		bool hover = false;
		bool pressed = false;
		ImGui::Separator();
		if (ImGui::Button("FadeIn")) {
			doFadeIn();
		}
		ImGui::SameLine();
		if (ImGui::Button("FadeOut")) {
			doFadeOut();
		}
		if (ImGui::Button("Idle")) { setState(B_IDLE); } ImGui::SameLine();
		if (ImGui::Button("Hover")) { setState(B_MOUSEOVER); }ImGui::SameLine();
		if (ImGui::Button("Pressed")) { setState(B_PRESSED); }
	}
}

void ComponentButtonUI::WhenPressed()
{
	int i = 0;
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
		WhenPressed();
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
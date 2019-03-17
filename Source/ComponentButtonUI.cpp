#include "ComponentButtonUI.h"
#include "Application.h"
#include "ComponentImageUI.h"
#include "ComponentRectTransform.h"
#include "GameObject.h"
#include "ResourceTexture.h"
#include "ModuleResourcesManager.h"
#include "ModuleUI.h"
#include "Material.h"

#include "Application.h"
#include "ModuleScripting.h"

#include "ImGui/imgui.h"

std::string openFileWID(bool isfile = false);

ComponentButtonUI::ComponentButtonUI(GameObject* parent) : Component(parent, UI_BUTTON)
{
	rectTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);
	image = (ComponentImageUI*)parent->getComponent(UI_IMAGE);

	image->setInspectorDraw(false);
}

ComponentButtonUI::ComponentButtonUI(JSON_Object * deff, GameObject * parent) : Component(parent, UI_BUTTON)
{
	rectTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);
	image = (ComponentImageUI*)parent->getComponent(UI_IMAGE);

	alpha = json_object_get_number(deff, "alpha");
	state = ButtonState((int)json_object_get_number(deff, "state"));

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
		idle = (ResourceTexture*)App->resources->getResource(texUUID);
	}
	texPath = json_object_dotget_string(deff, "textureHover");
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
		hover = (ResourceTexture*)App->resources->getResource(texUUID);
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
		pressed = (ResourceTexture*)App->resources->getResource(texUUID);
	}
	ChangeGOImage();

	JSON_Array* wren_calls = json_object_get_array(deff, "callbacks");

	for (int i = 0; i < json_array_get_count(wren_calls); i++) {
		JSON_Object* wren_call = json_array_get_object(wren_calls, i);
		callbacks.push_back(WrenCall(json_object_get_string(wren_call, "script_name"), json_object_get_string(wren_call, "method_name")));
	}

	image->setInspectorDraw(false);
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

bool ComponentButtonUI::DrawInspector(int id)
{
	if (ImGui::CollapsingHeader("UI Button"))
	{
		ButtonState state;//debug

		int w = 0; int h = 0;
		if (getResourceTexture(B_IDLE) != nullptr) {
			getResourceTexture(B_IDLE)->texture->getSize(w, h);
		}
		
		ImGui::Text("Idle texture data: \n x: %d\n y: %d", w, h);
		ImGui::Image(getResourceTexture(B_IDLE) != nullptr ? (void*)getResourceTexture(B_IDLE)->texture->getGLid() : (void*)App->gui->ui_textures[NO_TEXTURE]->getGLid(), ImVec2(128, 128));


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
		ImGui::Separator();

		int w3 = 0; int h3 = 0;
		if (getResourceTexture(B_MOUSEOVER) != nullptr) {
			getResourceTexture(B_MOUSEOVER)->texture->getSize(w3, h3);
		}

		ImGui::Text("Hover texture data: \n x: %d\n y: %d", w3, h3); 
		ImGui::Image(getResourceTexture(B_MOUSEOVER) != nullptr ? (void*)getResourceTexture(B_MOUSEOVER)->texture->getGLid() : (void*)App->gui->ui_textures[NO_TEXTURE]->getGLid(), ImVec2(128, 128));
		

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
		ImGui::Separator();

		

		int w2 = 0; int h2 = 0;
		if (getResourceTexture(B_PRESSED) != nullptr) {
			getResourceTexture(B_PRESSED)->texture->getSize(w2, h2);
		}

		ImGui::Text("Pressed texture data: \n x: %d\n y: %d", w2, h2);
		ImGui::Image(getResourceTexture(B_PRESSED) != nullptr ? (void*)getResourceTexture(B_PRESSED)->texture->getGLid() : (void*)App->gui->ui_textures[NO_TEXTURE]->getGLid(), ImVec2(128, 128));


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

		ImGui::Separator();
		if (ImGui::Button("FadeIn")) {
			doFadeIn();
		}
		ImGui::SameLine();
		if (ImGui::Button("FadeOut")) {
			doFadeOut();
		}
		
		// Functions

		ImGui::Text("Callbacks:");

		for (auto it = callbacks.begin(); it != callbacks.end(); it++) {
			std::string display = (*it).method_name + " (" + (*it).script_name + ")";
			ImGui::Text(display.c_str());
		}

		static bool display_methods = false;
		if (ImGui::Button("Add callback")) {
			display_methods = true;
		}

		if (display_methods) {
			WrenCall selected = App->scripting->DisplayMethods(parent, &display_methods);

			if (selected.method_name != "" && selected.script_name != "") { // A valid method has been selected
				callbacks.push_back(selected);
				display_methods = false;
			}
				
		}

		ImGui::SameLine();
		if (ImGui::Button("Remove callback")) {
			if (callbacks.size() > 0)
				callbacks.pop_back();
		}
	}
	return true;
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

	JSON_Value* wren_calls = json_value_init_array();
	
	for (auto it = callbacks.begin(); it != callbacks.end(); it++) {
		JSON_Value* wren_call = json_value_init_object();
		json_object_set_string(json_object(wren_call), "script_name", (*it).script_name.c_str());
		json_object_set_string(json_object(wren_call), "method_name", (*it).method_name.c_str());
		json_array_append_value(json_array(wren_calls), wren_call);
	}


	json_object_set_value(config, "callbacks", wren_calls);
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
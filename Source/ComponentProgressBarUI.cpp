#include "ComponentProgressBarUI.h"
#include "Application.h"
#include "GameObject.h"
#include "ComponentImageUI.h"
#include "ComponentRectTransform.h"
#include "ResourceTexture.h"
#include "ModuleResourcesManager.h"



ComponentProgressBarUI::ComponentProgressBarUI(GameObject * parent) : Component(parent, UI_PROGRESSBAR)
{
	intBarTransform = (ComponentRectTransform*)parent->getFirstChild()->getComponent(RECTTRANSFORM);
	barTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);
	
	intBar = (ComponentImageUI*)parent->getFirstChild()->getComponent(UI_IMAGE);
	bar = (ComponentImageUI*)parent->getComponent(UI_IMAGE);

	barTransform->setWidth(5.0f);
	barTransform->setHeight(1.2f);

	intBarTransform->setWidth(4.0f);
	intBarTransform->setHeight(1.f);
	intBarTransform->setDepth(-0.1f);

	initWidth = intBarTransform->getWidth();
}


ComponentProgressBarUI::ComponentProgressBarUI(JSON_Object * deff, GameObject * parent) : Component(parent, UI_PROGRESSBAR)
{
	
	barTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);

	bar = (ComponentImageUI*)parent->getComponent(UI_IMAGE);
	
	initWidth = json_object_get_number(deff, "initWidth");
	percent = json_object_get_number(deff, "percent");

	const char* texPath = json_object_dotget_string(deff, "textureBar");
	if (texPath && strcmp(texPath, "missing reference") != 0) {
		uint uuid = App->resources->getResourceUuid(texPath);
		intTexBar = (ResourceTexture*)App->resources->getResource(uuid);
	}
	texPath = json_object_dotget_string(deff, "textureBarInterior");
	if (texPath && strcmp(texPath, "missing reference") != 0) {
		uint uuid = App->resources->getResourceUuid(texPath);
		texBar = (ResourceTexture*)App->resources->getResource(uuid);
	}

}

ComponentProgressBarUI::~ComponentProgressBarUI()
{ 
	intBar = nullptr;
	bar = nullptr;
	intTexBar = nullptr;
	texBar = nullptr;
}

bool ComponentProgressBarUI::Update(float dt)
{
	if(!intBar){ intBar = (ComponentImageUI*)parent->getFirstChild()->getComponent(UI_IMAGE); }
	if (!intBarTransform) { intBarTransform = (ComponentRectTransform*)parent->getFirstChild()->getComponent(RECTTRANSFORM); }

	return true;
}

 void ComponentProgressBarUI::setPercent(float _percent)
 {
	 percent = _percent;
	 intBarTransform->setWidth(initWidth*percent / 100);
}


void ComponentProgressBarUI::setPos(float2 _pos)
{
	pos = _pos;
	barTransform->setPos(pos);

}

void ComponentProgressBarUI::setInteriorWidth(float width)
{
	initWidth = width;
	intBarTransform->setWidth(initWidth*percent / 100);
}

void ComponentProgressBarUI::setInteriorDepth(float depth)
{
	if (intBarTransform)
		intBarTransform->setDepth(depth);
}
const float ComponentProgressBarUI::getInteriorDepth(){
	if (intBarTransform) 
		return intBarTransform->getDepth();
}



void ComponentProgressBarUI::setResourceTexture(ResourceTexture * tex)
{
	texBar = tex; 
	bar->setResourceTexture(texBar);
}

void ComponentProgressBarUI::setResourceTextureInterior(ResourceTexture * tex)
{
	intTexBar = tex; 
	intBar->setResourceTexture(intTexBar);
}

void ComponentProgressBarUI::Save(JSON_Object * config)
{
	json_object_set_string(config, "type", "UIprogress_bar");
	json_object_set_number(config, "initWidth", initWidth);
	json_object_set_number(config, "percent", percent);

	std::string texName = std::string("missing_reference");
	if (intTexBar) {  //If it has a texture
		texName = intTexBar->asset;
	}
	json_object_dotset_string(config, "textureBar", texName.c_str());
	texName = "missing_reference";

	if (texBar) {
		texName = texBar->asset;
	}
	json_object_dotset_string(config, "textureBarInterior", texName.c_str());
	texName = "missing_reference";
}

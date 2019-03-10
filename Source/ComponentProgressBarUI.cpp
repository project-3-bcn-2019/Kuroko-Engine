#include "ComponentProgressBarUI.h"
#include "GameObject.h"
#include "ComponentImageUI.h"
#include "ComponentRectTransform.h"



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

	initWidth = intBarTransform->getWidth();
}


ComponentProgressBarUI::ComponentProgressBarUI(JSON_Object * deff, GameObject * parent) : Component(parent, UI_PROGRESSBAR)
{
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
	json_object_set_string(config, "type", "progress_bar");
}

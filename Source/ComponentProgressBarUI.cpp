#include "ComponentProgressBarUI.h"
#include "GameObject.h"
#include "ComponentImageUI.h"
#include "ComponentRectTransform.h"



ComponentProgressBarUI::ComponentProgressBarUI(GameObject * parent) : Component(parent, UI_PROGRESSBAR)
{
	intBarTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);
	barTransform = (ComponentRectTransform*)parent->getParent()->getComponent(RECTTRANSFORM);
	
	intBar = (ComponentImageUI*)parent->getComponent(UI_IMAGE);
	bar = (ComponentImageUI*)parent->getParent()->getComponent(UI_IMAGE);

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

void ComponentProgressBarUI::setOffset(float2 _offset) 
{
	offset = _offset;
	intBarTransform->setPos(intBarTransform->getLocalPos() + offset);
}

void ComponentProgressBarUI::Save(JSON_Object * config)
{
	json_object_set_string(config, "type", "progress_bar");
}

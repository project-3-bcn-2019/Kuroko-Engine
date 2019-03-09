#include "ComponentProgressBarUI.h"
#include "GameObject.h"
#include "ComponentImageUI.h"
#include "ComponentRectTransform.h"



ComponentProgressBarUI::ComponentProgressBarUI(GameObject * parent) : Component(parent, UI_PROGRESSBAR)
{
	barTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);
	intBarTransform = (ComponentRectTransform*)parent->getParent()->getComponent(RECTTRANSFORM);
	
	intBar = (ComponentImageUI*)parent->getComponent(UI_IMAGE);
	bar = (ComponentImageUI*)parent->getParent()->getComponent(UI_IMAGE);

	barTransform->setWidth(5.0f);
	barTransform->setHeight(1.2f);

	intBarTransform->setWidth(4.0f);
	intBarTransform->setHeight(1.f);

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

void ComponentProgressBarUI::Draw() const
{
}

void ComponentProgressBarUI::Save(JSON_Object * config)
{
	json_object_set_string(config, "type", "progress_bar");
}

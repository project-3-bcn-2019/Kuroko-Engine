#include "ComponentProgressBarUI.h"




ComponentProgressBarUI::ComponentProgressBarUI(GameObject * parent) : Component(parent, UI_PROGRESSBAR)
{
}


ComponentProgressBarUI::ComponentProgressBarUI(JSON_Object * deff, GameObject * parent) : Component(parent, UI_PROGRESSBAR)
{
}

ComponentProgressBarUI::~ComponentProgressBarUI()
{
}

bool ComponentProgressBarUI::Update(float dt)
{
	return false;
}

void ComponentProgressBarUI::Draw() const
{
}

void ComponentProgressBarUI::Save(JSON_Object * config)
{
	json_object_set_string(config, "type", "progress_bar");
}

#ifndef _COMPONENTPROGRESSUI_
#define _COMPONENTPROGRESSUI_
#include "Component.h"


class ComponentRectTransform;
class ComponentImageUI;
class ResourceTexture;

class ComponentProgressBarUI :
	public Component
{
public:
	ComponentProgressBarUI(GameObject* parent);
	ComponentProgressBarUI(JSON_Object* deff, GameObject* parent);
	~ComponentProgressBarUI();

	bool Update(float dt)override;

	inline void setPercent(float _percent) { percent = _percent; }
	inline const float getPercent() { return percent; }

	void Save(JSON_Object* config) override;

private:

	float percent;

	ComponentRectTransform * barTransform = nullptr;
	ComponentRectTransform * intBarTransform = nullptr;

	ComponentImageUI* bar = nullptr;
	ComponentImageUI* intBar = nullptr;

	ResourceTexture* texBar = nullptr;
	ResourceTexture* intTexBar = nullptr;
};


#endif
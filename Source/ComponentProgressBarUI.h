#ifndef _COMPONENTPROGRESSUI_
#define _COMPONENTPROGRESSUI_
#include "Component.h"
#include "MathGeoLib/Math/float2.h"


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

	 void setPercent(float _percent);
	inline const float getPercent() { return percent; }

	void setPos(float2 pos);
	inline const float2 getPos() { return pos; }

	void setInteriorWidth(float width);
	inline const float getInteriorWidth() { return initWidth; }

	inline const ResourceTexture* getResourceTexture() { return texBar; }
	void setResourceTexture(ResourceTexture* tex);
	inline void DeassignTexture() { texBar = nullptr; }

	inline const ResourceTexture* getResourceTextureInterior() { return intTexBar; }
	void setResourceTextureInterior(ResourceTexture* tex);
	inline void DeassignTextureInterior() { intTexBar = nullptr; }

	

	void Save(JSON_Object* config) override;

private:

	float2 pos = float2(0.f, 0.f);
	float percent = 100.f;
	float initWidth;

	ComponentRectTransform * barTransform = nullptr;
	ComponentRectTransform * intBarTransform = nullptr;

	ComponentImageUI* bar = nullptr;
	ComponentImageUI* intBar = nullptr;

	ResourceTexture* texBar = nullptr;
	ResourceTexture* intTexBar = nullptr;
};


#endif
#ifndef _COMPONENTIMAGEUI_
#define _COMPONENTIMAGEUI_
#include "Component.h"
#include "MathGeoLib/Math/float2.h"

class ComponentRectTransform;
class ResourceTexture;

class ComponentImageUI : public Component
{
public:
	ComponentImageUI(GameObject* parent);
	ComponentImageUI(JSON_Object* deff, GameObject* parent);
	~ComponentImageUI();

	bool Update(float dt)override;
	void Draw() const override;
	bool DrawInspector(int id = 0) override;

	void Save(JSON_Object* config) override;
	
	inline const ResourceTexture* getResourceTexture() { return texture; }
	inline void setResourceTexture(ResourceTexture* tex) { texture = tex; }
	
	inline void DeassignTexture() { texture = nullptr; }


	inline void doFadeIn() { alpha = 0.0f; fadingIn = true; }
	inline void doFadeOut() { alpha = 1.0f; fadingOut = true; }
	
	float alpha = 1.0f; //not functional yet
private:
	ComponentRectTransform* rectTransform = nullptr;	
	ResourceTexture* texture = nullptr;
	float2* texCoords = nullptr;

	void FadeIn();
	void FadeOut();
	bool fadingIn = false;
	bool fadingOut = false;
};

#endif
#ifndef _COMPONENTCANVAS_
#define _COMPONENTCANVAS_
#include "Component.h"
#include "MathGeoLib/Math/float2.h"

class ComponentRectTransform;

class ComponentCanvas :	public Component
{
public:
	ComponentCanvas(GameObject* parent);
	ComponentCanvas(JSON_Object * deff, GameObject * parent);
	~ComponentCanvas();

	bool Update(float dt) override;
	void Draw() const override;
	void DrawInspector(int id = 0);
	void Save(JSON_Object* config) override;

	void setResolution(float2 resolution);	
	inline const float2 getResolution();

	bool draw_cross = false;

private:	

	//float2 _resolution = float2::zero;
	ComponentRectTransform* rectTransform = nullptr;
	
};

#endif
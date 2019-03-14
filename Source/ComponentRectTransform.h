#ifndef _COMPONENTRECTTRANSFORM_
#define _COMPONENTRECTTRANSFORM_

#include "Component.h"
#include "MathGeoLib/Math/float2.h"
#include "MathGeoLib/Math/float3.h"

class RectTransform {
public:
	float2 local = float2::zero;
	float2 global = float2::zero;
	float width = 0.0f;
	float height = 0.0f;
	float2 anchor = float2(0.f, 0.f);
	int vertexID = -1;
	float3* vertex = nullptr;
	float depth = 0.0f;
	
};

class ComponentRectTransform : public Component
{
public:
	ComponentRectTransform(GameObject* parent);
	ComponentRectTransform(JSON_Object * deff, GameObject * parent);
	~ComponentRectTransform();

	bool Update(float dt) override;
	void Draw() const override;
	bool DrawInspector(int id = 0) override;
	const float2 getMid() const;
	void Save(JSON_Object* config) override;


	void setPos(float2 pos);
	void setWidth(float width);
	void setHeight(float height);
	void setToMid();

	inline const float2 getAnchor() { return rect.anchor; }
	inline const float2 getLocalPos() { return rect.local; }
	inline const float2 getGlobalPos() { return rect.global; }
	inline const float getWidth() { return rect.width; }
	inline const float getHeight() { return rect.height; }
	inline const float getDepth() { return rect.depth; }
	inline const int GetVertexID() { return rect.vertexID; }

	inline void setGlobalPos(float2 pos) { rect.global = pos; }
	inline void setDepth(float _depth) { rect.depth = _depth; }

	bool debug_draw = false;

private:
	inline void setLocalPos(float2 pos) { rect.local = pos; }
	void GenBuffer();
	void UpdateGlobalMatrixRecursive(ComponentRectTransform * rect);
	void UpdateAnchorPos();
	RectTransform rect;
};


#endif
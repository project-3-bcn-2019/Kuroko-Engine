#ifndef _COMPONENTAABB_
#define _COMPONENTAABB_

#include "Component.h"
#include "Color.h"

#include <list>

#include "MathGeoLib\Geometry\AABB.h"
#include "MathGeoLib\Geometry\OBB.h"

class ComponentTransform;

class ComponentAABB : public Component
{
public:
	ComponentAABB(GameObject* parent);
	~ComponentAABB();

	bool Update(float dt);
	void Draw() const;
	void DrawInspector(int id = 0);
	void Reload();

	math::AABB* getAABB() const { return aabb; };
	math::OBB* getOBB() const { return obb; };

	void DrawAABB() const;
	void DrawOBB() const;

	void Save(JSON_Object* config);


public:
	bool draw_aabb = false;
	bool draw_obb = false;


private: 

	math::AABB* aabb = nullptr;
	math::OBB* obb = nullptr;

	ComponentTransform* transform = nullptr;

};
#endif
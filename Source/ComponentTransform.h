#ifndef _COMPONENT_TRANSFORM
#define _COMPONENT_TRANSFORM

#include "Component.h"


#include "MathGeoLib\Math\float3.h"
#include "MathGeoLib\Math\Quat.h"
#include "MathGeoLib\Math\float4x4.h"

class Transform;

enum TransformMode { LOCAL, GLOBAL};

class ComponentTransform : public Component {

	friend class ModuleUI;
	friend class ModuleCamera3D;
public:

	ComponentTransform(GameObject* parent, const Quat& rot, const float3& pos, const float3& scl);
	ComponentTransform(GameObject* parent, const float3& euler_axis, const float3& pos, const float3& scl);
	ComponentTransform(GameObject* parent, const ComponentTransform& transform);
	ComponentTransform(JSON_Object* deff, GameObject* parent);
	ComponentTransform(GameObject* parent);

	~ComponentTransform();

	bool Update(float dt); 
	void Draw() const;

	Transform* getInheritedTransform();

	void setMode(TransformMode mode) { this->mode = mode; };
	TransformMode getMode() { return mode; };

	void Save(JSON_Object* config);


	void LocalToGlobal();
	void GlobalToLocal();
public:

	Transform* local = nullptr;
	Transform* global = nullptr;
	bool draw_axis = false;
	bool constraints[3][3] = { {false, false, false}, {false, false, false}, {false, false, false} };
private:

	TransformMode mode = LOCAL;

};
#endif
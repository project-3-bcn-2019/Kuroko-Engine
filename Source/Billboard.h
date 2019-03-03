#ifndef _BILLBOARD_H_
#define _BILLBOARD_H_

#include "Color.h"
#include "MathGeoLib\Math\float3.h"
#include "Parson\parson.h"

enum Alignment {
	SCREEN_ALIGN,
	WORLD_ALIGN,
	AXIAL_ALIGN
};

class Component;
class Material;
class Camera;
class Transform;
class Mesh;
class Particle;

class Billboard
{
	friend class ModuleUI;
public:
	Billboard(Component* p_component, Mesh* _mesh,  Material* mat = nullptr);
	Billboard(Component* p_component, Mesh* _mesh, JSON_Object* deff);
	Billboard(Component* p_component, const Billboard& other);
	~Billboard();

	void Draw() const;
	void Save(JSON_Object* json);
	void UpdateTransform(float3 pos, float3 scl);
	void UpdateFromParticle(const Particle& particle);

	Material* getMaterial() const { return material; };
	void setMaterial(Material* mat) { material = mat; };

	void FaceCamera();


	bool useColor = true;
	Color color = White;
	Camera* camera = nullptr;

private:

	void WorldAlign();
	void ScreenAlign();
	void AxialAlign();

	Material* material = nullptr;
	Mesh* mesh = nullptr;
	Alignment alignment = SCREEN_ALIGN;
	Transform* transform = nullptr;

	Component* parent_component = nullptr;
	bool wireframe = false;
};

#endif
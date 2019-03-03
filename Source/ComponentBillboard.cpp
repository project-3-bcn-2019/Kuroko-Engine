#include "ComponentBillboard.h"
#include "Application.h"

#include "GameObject.h"
#include "Camera.h"
#include "ComponentTransform.h"
#include "Mesh.h"
#include "Transform.h"
#include "ModuleCamera3D.h"
#include "ModuleScene.h"
#include "ComponentAABB.h"
#include "Material.h"
#include "ResourceTexture.h"
#include "ModuleResourcesManager.h"


ComponentBillboard::ComponentBillboard(GameObject* parent, Material* mat) : Component(parent, BILLBOARD)
{
	transform = (ComponentTransform*)getParent()->getComponent(TRANSFORM);
	mesh = new Mesh(Primitive_Plane);
	billboard = new Billboard(this, mesh, mat);
}

ComponentBillboard::ComponentBillboard(JSON_Object* deff, GameObject* parent) : Component(parent, BILLBOARD)
{
	transform = (ComponentTransform*)getParent()->getComponent(TRANSFORM);

	mesh = new Mesh(Primitive_Plane);
	billboard = new Billboard(this, mesh, deff);
}


ComponentBillboard::~ComponentBillboard()
{
	if (billboard)
		delete billboard;
}

bool ComponentBillboard::Update(float dt)
{
	billboard->camera = App->camera->current_camera;
	billboard->FaceCamera();
	billboard->UpdateTransform(transform->global->getPosition(), transform->global->getScale());

	return true;
}

void ComponentBillboard::Draw() const
{
	if (billboard->useColor)	mesh->tint_color = billboard->color;
	else						mesh->tint_color = White;

	billboard->Draw();
}

void ComponentBillboard::Save(JSON_Object* config)
{
	// Determine the type of the mesh
	// Component has two strings, one for mesh name, and another for diffuse texture name
	json_object_set_string(config, "type", "billboard");
	billboard->Save(config);
}

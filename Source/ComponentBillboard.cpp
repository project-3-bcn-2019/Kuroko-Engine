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
#include "ModuleUI.h"
#include "Material.h"

std::string openFileWID(bool isfile = false);


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

bool ComponentBillboard::DrawInspector(int id)
{
	if (ImGui::CollapsingHeader("Billboard"))
	{
		if (Material* material = billboard->getMaterial())
		{
			static int preview_size = 128;
			ImGui::Text("Id: %d", material->getId());
			ImGui::SameLine();
			if (ImGui::Button("remove material"))
			{
				delete billboard->getMaterial();
				billboard->setMaterial(nullptr);
				ImGui::TreePop();
			}

			ImGui::Text("Preview size");
			ImGui::SameLine();
			if (ImGui::Button("64")) preview_size = 64;
			ImGui::SameLine();
			if (ImGui::Button("128")) preview_size = 128;
			ImGui::SameLine();
			if (ImGui::Button("256")) preview_size = 256;

			Texture* texture = nullptr;
			if (ResourceTexture* tex_res = (ResourceTexture*)App->resources->getResource(material->getTextureResource(DIFFUSE)))
				texture = tex_res->texture;


			ImGui::Image(texture ? (void*)texture->getGLid() : (void*)App->gui->ui_textures[NO_TEXTURE]->getGLid(), ImVec2(preview_size, preview_size));
			ImGui::SameLine();

			int w = 0; int h = 0;
			if (texture)
				texture->getSize(w, h);

			ImGui::Text("texture data: \n x: %d\n y: %d", w, h);

			//if (ImGui::Button("Load checkered##Dif: Load checkered"))
			//	material->setCheckeredTexture(DIFFUSE);
			//ImGui::SameLine()
		}
		else
			ImGui::TextWrapped("No material assigned");

		if (ImGui::Button("Load material(from asset folder)##Billboard: Load"))
		{
			std::string texture_path = openFileWID();
			uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
			if (new_resource != 0) {
				App->resources->assignResource(new_resource);

				if (Material* material = billboard->getMaterial())
					App->resources->deasignResource(material->getTextureResource(DIFFUSE));
				else
					billboard->setMaterial(new Material());

				billboard->getMaterial()->setTextureResource(DIFFUSE, new_resource);
			}
		}

		ImGui::Checkbox("Use Color", &billboard->useColor);

		if (billboard->useColor)
		{
			static bool draw_colorpicker = false;
			static Color reference_color =billboard->color;
			static GameObject* last_selected = getParent();

			std::string label = getParent()->getName() + " color picker";

			if (last_selected != getParent())
				reference_color = billboard->color;

			ImGui::SameLine();
			if (ImGui::ColorButton((label + "button").c_str(), ImVec4(billboard->color.r, billboard->color.g, billboard->color.b, billboard->color.a)))
				draw_colorpicker = !draw_colorpicker;

			if (draw_colorpicker)
				App->gui->DrawColorPickerWindow(label.c_str(), (Color*)&billboard->color, &draw_colorpicker, (Color*)&reference_color);
			else
				reference_color = billboard->color;

			last_selected = getParent();
		}

		if (ImGui::CollapsingHeader("Alignement"))
		{
			if (ImGui::Selectable("Screen aligned", billboard->alignment == SCREEN_ALIGN))
				billboard->alignment = SCREEN_ALIGN;

			if (ImGui::Selectable("World aligned", billboard->alignment == WORLD_ALIGN))
				billboard->alignment = WORLD_ALIGN;

			if (ImGui::Selectable("Axis aligned", billboard->alignment == AXIAL_ALIGN))
				billboard->alignment = AXIAL_ALIGN;
		}

		/*if (ImGui::Button("Remove##Remove billboard"))
			return false;*/
	}
	return true;
}

void ComponentBillboard::Save(JSON_Object* config)
{
	// Determine the type of the mesh
	// Component has two strings, one for mesh name, and another for diffuse texture name
	json_object_set_string(config, "type", "billboard");
	billboard->Save(config);
}

#include "ComponentImageUI.h"
#include "Application.h"
#include "GameObject.h"
#include "glew-2.1.0\include\GL\glew.h"
#include "MathGeoLib/Math/float4x4.h"
#include "MathGeoLib/Math/Quat.h"
#include "ComponentRectTransform.h"

#include "ResourceTexture.h"
#include "Material.h"
#include "ModuleResourcesManager.h"
#include "ModuleUI.h"

#include "ImGui/imgui.h"

std::string openFileWID(bool isfile = false);

ComponentImageUI::ComponentImageUI(GameObject* parent) : Component(parent, UI_IMAGE)
{
	rectTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);

	static const float uvs[] = {
		
		0, 1,
		1, 1,
		1, 0,
		0, 0
	};

	texCoords = new float2[4];
	memcpy(texCoords, uvs, sizeof(float2) * 4);
}

ComponentImageUI::ComponentImageUI(JSON_Object * deff, GameObject * parent) : Component(parent, UI_IMAGE)
{
	rectTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);

	static const float uvs[] = {

		0, 1,
		1, 1,
		1, 0,
		0, 0
	};

	texCoords = new float2[4];
	memcpy(texCoords, uvs, sizeof(float2) * 4);

	alpha = json_object_get_number(deff, "alpha");
	const char* texPath = json_object_dotget_string(deff, "textureName");

	if (texPath) {
		if (strcmp(texPath, "missing reference") != 0) {
			uint uuid = App->resources->getResourceUuid(texPath);
			App->resources->assignResource(uuid);
			texture = (ResourceTexture*)App->resources->getResource(uuid);
			
		}
	}

}


ComponentImageUI::~ComponentImageUI()
{
	rectTransform = nullptr;
	RELEASE_ARRAY(texCoords);
	if (texture) {
		App->resources->deasignResource(texture->uuid);
		texture = nullptr;
	}
	
}

bool ComponentImageUI::Update(float dt)
{

	if (fadingOut) {
		FadeOut();
	}
	if (fadingIn) {
		FadeIn();
	}

	return true;
}

void ComponentImageUI::Draw() const
{
	glPushMatrix();
	float4x4 globalMat;
	globalMat = float4x4::FromTRS(float3(rectTransform->getGlobalPos().x, rectTransform->getGlobalPos().y, rectTransform->getDepth()), Quat(0, 0, 0, 0), float3(rectTransform->getWidth(), rectTransform->getHeight(), 0));
	glMultMatrixf(globalMat.Transposed().ptr());

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0); //resets the buffer
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0);
	glColor4f(1.0, 1.0, 1.0, alpha);
	glLineWidth(4.0f);

	if (texture) { 
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture->texture->getGLid());
		glTexCoordPointer(2, GL_FLOAT, 0, &(texCoords[0]));
		
	}
	glBindBuffer(GL_ARRAY_BUFFER, rectTransform->GetVertexID());
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glDrawArrays(GL_QUADS, 0, 4);

	glLineWidth(1.0f);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0); //resets the buffer
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	if (texture) { glDisable(GL_TEXTURE_2D); }
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glPopMatrix();

}

bool ComponentImageUI::DrawInspector(int id)
{
	if (ImGui::CollapsingHeader("UI Image"))
	{

		ImGui::Image(getResourceTexture() != nullptr ? (void*)getResourceTexture()->texture->getGLid() : (void*)App->gui->ui_textures[NO_TEXTURE]->getGLid(), ImVec2(128, 128));
		ImGui::SameLine();

		int w = 0; int h = 0;
		if (getResourceTexture() != nullptr) {
			getResourceTexture()->texture->getSize(w, h);
		}

		ImGui::Text("texture data: \n x: %d\n y: %d", w, h);

		ImGui::SliderFloat("Alpha", &alpha, 0.0f, 1.0f);

		if (ImGui::Button("Load(from asset folder)##Dif: Load"))
		{
			std::string texture_path = openFileWID();
			uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
			if (new_resource != 0) {
				App->resources->assignResource(new_resource);
				if (getResourceTexture() != nullptr)
					App->resources->deasignResource(getResourceTexture()->uuid);
				setResourceTexture((ResourceTexture*)App->resources->getResource(new_resource));
			}
		}
	}
	return true;
}

void ComponentImageUI::Save(JSON_Object * config)
{
	json_object_set_string(config, "type", "UIimage");
	json_object_set_number(config, "alpha", alpha);

	std::string texName = std::string("missing_reference");
	if (texture) {  //If it has a texture
		texName = texture->asset;		
	}
	json_object_dotset_string(config, "textureName",texName.c_str());


}

void ComponentImageUI::FadeIn()
{
	alpha += DELTA_ALPHA;
	if (alpha >= 1.0f) {
		fadingIn = false;
		alpha = 1.0f;		
	}
}

void ComponentImageUI::FadeOut()
{
	alpha -= DELTA_ALPHA;
	if (alpha <= 0.0f) {
		fadingOut = false;
		alpha = 0.0f;
	}
}
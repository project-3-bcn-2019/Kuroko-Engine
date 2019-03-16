#include "ComponentProgressBarUI.h"
#include "Application.h"
#include "GameObject.h"
#include "ComponentImageUI.h"
#include "ComponentRectTransform.h"
#include "ResourceTexture.h"
#include "ModuleResourcesManager.h"
#include "ModuleUI.h"
#include "Material.h"

#include "ImGui/imgui.h"

std::string openFileWID(bool isfile = false);


ComponentProgressBarUI::ComponentProgressBarUI(GameObject * parent) : Component(parent, UI_PROGRESSBAR)
{
	intBarTransform = (ComponentRectTransform*)parent->getFirstChild()->getComponent(RECTTRANSFORM);
	barTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);
	
	intBar = (ComponentImageUI*)parent->getFirstChild()->getComponent(UI_IMAGE);
	bar = (ComponentImageUI*)parent->getComponent(UI_IMAGE);

	barTransform->setWidth(5.0f);
	barTransform->setHeight(1.2f);

	intBarTransform->setWidth(4.0f);
	intBarTransform->setHeight(1.f);
	intBarTransform->setDepth(-0.1f);

	initialWidth = intBarTransform->getWidth();

	intBarTransform->setInspectorDraw(false);
	barTransform->setInspectorDraw(false);
	intBar->setInspectorDraw(false);
	bar->setInspectorDraw(false);
}


ComponentProgressBarUI::ComponentProgressBarUI(JSON_Object * deff, GameObject * parent) : Component(parent, UI_PROGRESSBAR)
{
	
	barTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);

	bar = (ComponentImageUI*)parent->getComponent(UI_IMAGE);
	
	initialWidth = json_object_get_number(deff, "initWidth");
	percent = json_object_get_number(deff, "percent");

	const char* texPath = json_object_dotget_string(deff, "textureBar");
	if (texPath && strcmp(texPath, "missing reference") != 0) {
		uint uuid = App->resources->getResourceUuid(texPath);
		intTexBar = (ResourceTexture*)App->resources->getResource(uuid);
	}
	texPath = json_object_dotget_string(deff, "textureBarInterior");
	if (texPath && strcmp(texPath, "missing reference") != 0) {
		uint uuid = App->resources->getResourceUuid(texPath);
		texBar = (ResourceTexture*)App->resources->getResource(uuid);
	}


	
	bar->setInspectorDraw(false);
}

ComponentProgressBarUI::~ComponentProgressBarUI()
{ 
	intBar = nullptr;
	bar = nullptr;
	intTexBar = nullptr;
	texBar = nullptr;
}

bool ComponentProgressBarUI::Update(float dt)
{
	if(!intBar){ intBar = (ComponentImageUI*)parent->getFirstChild()->getComponent(UI_IMAGE); intBar->setInspectorDraw(false);}
	if(!intBarTransform) { intBarTransform = (ComponentRectTransform*)parent->getFirstChild()->getComponent(RECTTRANSFORM); }

	return true;
}

bool ComponentProgressBarUI::DrawInspector(int id)
{
	if (ImGui::CollapsingHeader("UI Progress Bar"))
	{
		float2 position = getPos();
		static float percent = getPercent();
		static float width = barTransform->getWidth();
		static float height = barTransform->getHeight();
		static float depth = barTransform->getDepth();

		float2 positionInt = intBarTransform->getGlobalPos();
		static float heightInt = intBarTransform->getHeight();
		static float widthInt = getInteriorWidth();
		static float depthInt =getInteriorDepth();

		ImGui::Text("Percent:");
		ImGui::DragFloat("##d", (float*)&percent, 1, 0, 100); {setPercent(percent); }
		ImGui::Separator();
		ImGui::Text("BAR:");
		ImGui::Text("Position:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
		if (ImGui::DragFloat2("##ps", (float*)&position, 0.5f)) { setPos(position); }
		ImGui::Text("Width:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
		if (ImGui::DragFloat("##wi", (float*)&width, 0.5f)) { barTransform->setWidth(width); }
		ImGui::Text("Height:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
		if (ImGui::DragFloat("##hi", (float*)&height, 0.5f)) { barTransform->setHeight(height); }
		ImGui::Text("Depth:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
		if (ImGui::DragFloat("##d2", (float*)&depth, 0.1f)) { barTransform->setDepth(depth); }

		int w = 0; int h = 0;
		if (getResourceTexture() != nullptr) {
			getResourceTexture()->texture->getSize(w, h);
		}


		ImGui::Text("Texture data: \n x: %d\n y: %d", w, h);
		ImGui::Image(getResourceTexture() != nullptr ? (void*)getResourceTexture()->texture->getGLid() : (void*)App->gui->ui_textures[NO_TEXTURE]->getGLid(), ImVec2(128, 128));


		if (ImGui::Button("Load(from asset folder)##Dif: Loadtex"))
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

		

		ImGui::Separator();

		ImGui::Text("INTERIOR BAR:");
		ImGui::Text("Position:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
		if (ImGui::DragFloat2("##p2s", (float*)&positionInt, 0.5f)) { intBarTransform->setPos(positionInt); }

		ImGui::Text("Width:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
		if (ImGui::DragFloat("##wi2", (float*)&widthInt, 0.5f)) { setInteriorWidth(widthInt); }
		ImGui::Text("Height:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
		if (ImGui::DragFloat("##hi2", (float*)&heightInt, 0.5f)) { 
			intBarTransform->setHeight(heightInt); }
		ImGui::Text("Depth:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
		if (ImGui::DragFloat("##dp2", (float*)&depthInt, 0.1f)) { setInteriorDepth(depthInt); }
		
		
		ImGui::Text("Width: %f", initialWidth);
		

		int w2 = 0; int h2 = 0;
		if (getResourceTextureInterior() != nullptr) {
			getResourceTextureInterior()->texture->getSize(w2, h2);
		}


		ImGui::Text("Texture data: \n x: %d\n y: %d", w2, h2);
		ImGui::Image(getResourceTextureInterior() != nullptr ? (void*)getResourceTextureInterior()->texture->getGLid() : (void*)App->gui->ui_textures[NO_TEXTURE]->getGLid(), ImVec2(128, 128));


		if (ImGui::Button("Load(from asset folder)##Dif: Loadtex2"))
		{
			std::string texture_path = openFileWID();
			uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
			if (new_resource != 0) {
				App->resources->assignResource(new_resource);
				if (getResourceTextureInterior() != nullptr)
					App->resources->deasignResource(getResourceTextureInterior()->uuid);
				setResourceTextureInterior((ResourceTexture*)App->resources->getResource(new_resource));
			}
		}

		ImGui::SameLine();
	}
	return true;
}

 void ComponentProgressBarUI::setPercent(float _percent)
 {
	 percent = _percent;
	 intBarTransform->setWidth(initialWidth*percent / 100);
}


inline void ComponentProgressBarUI::setPos(float2 _pos)
{
	barTransform->setPos(_pos);
}
void ComponentProgressBarUI::setWidth(float width)
{
	barTransform->setWidth(width);
	intBarTransform->setWidth(initialWidth*percent / 100);
}

inline const float2 ComponentProgressBarUI::getPos()
{
	return barTransform->getLocalPos();
}

void ComponentProgressBarUI::setInteriorWidth(float width)
{
	initialWidth = width;
	intBarTransform->setWidth(initialWidth*percent / 100);
}

void ComponentProgressBarUI::setInteriorDepth(float depth)
{
	if (intBarTransform)
		intBarTransform->setDepth(depth);
}
const float ComponentProgressBarUI::getInteriorDepth(){
	if (intBarTransform) 
		return intBarTransform->getDepth();
}



void ComponentProgressBarUI::setResourceTexture(ResourceTexture * tex)
{
	texBar = tex; 
	bar->setResourceTexture(texBar);
}

void ComponentProgressBarUI::setResourceTextureInterior(ResourceTexture * tex)
{
	intTexBar = tex; 
	intBar->setResourceTexture(intTexBar);
}

void ComponentProgressBarUI::Save(JSON_Object * config)
{
	json_object_set_string(config, "type", "UIprogress_bar");
	json_object_set_number(config, "initWidth", initialWidth);
	json_object_set_number(config, "percent", percent);

	std::string texName = std::string("missing_reference");
	if (intTexBar) {  //If it has a texture
		texName = intTexBar->asset;
	}
	json_object_dotset_string(config, "textureBar", texName.c_str());
	texName = "missing_reference";

	if (texBar) {
		texName = texBar->asset;
	}
	json_object_dotset_string(config, "textureBarInterior", texName.c_str());
	texName = "missing_reference";
}

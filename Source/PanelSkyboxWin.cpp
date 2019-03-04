#include "PanelSkyboxWin.h"
#include "Application.h"
#include "ModuleScene.h"
#include "Skybox.h"

PanelSkyboxWin::PanelSkyboxWin(const char * name, bool active): Panel(name)
{
}

PanelSkyboxWin::~PanelSkyboxWin()
{
}

void PanelSkyboxWin::Draw()
{
	//ImGui::Begin("Skybox", &active);

	//if (Skybox* skybox = App->scene->skybox)
	//{
	//	ImGui::Checkbox("active##skybox active", &skybox->active);

	//	static float new_distance = skybox->distance;
	//	ImGui::PushItemWidth(ImGui::GetWindowWidth() - 10);
	//	if (ImGui::SliderFloat("##skybox distance", &new_distance, 1.0f, 5000.0f, "Distance: %.0f"))
	//		skybox->setDistance(new_distance);

	//	ImGui::Checkbox("color mode##skybox color mode", &skybox->color_mode);

	//	if (skybox->color_mode)
	//	{
	//		static bool draw_colorpicker = false;
	//		static Color reference_color = skybox->color;

	//		ImGui::SameLine();
	//		if (ImGui::ColorButton("##skybox_color", ImVec4(skybox->color.r, skybox->color.g, skybox->color.b, skybox->color.a)))
	//			draw_colorpicker = !draw_colorpicker;

	//		if (draw_colorpicker)
	//			DrawColorPickerWindow("Skybox color picker", (Color*)&skybox->color, &draw_colorpicker, (Color*)&reference_color);
	//		else
	//			reference_color = skybox->color;
	//	}
	//	else
	//	{
	//		if (ImGui::Button("Clear all textures##clear all skybox textures"))
	//		{
	//			for (int i = 0; i < 6; i++)
	//				skybox->removeTexture((Direction)i);
	//		}

	//		for (int i = 0; i < 6; i++)
	//		{
	//			char* label = "";
	//			Texture* texture = nullptr;

	//			switch (i)
	//			{
	//			case LEFT:	label = "left_texture";		texture = skybox->getTexture(LEFT);		break;
	//			case RIGHT: label = "right_texture";	texture = skybox->getTexture(RIGHT);	break;
	//			case UP:	label = "up_texture";		texture = skybox->getTexture(UP);		break;
	//			case DOWN:	label = "down_texture";		texture = skybox->getTexture(DOWN);		break;
	//			case FRONT: label = "front_texture";	texture = skybox->getTexture(FRONT);	break;
	//			case BACK:	label = "back_texture";		texture = skybox->getTexture(BACK);		break;
	//			}

	//			if (ImGui::TreeNode(label))
	//			{
	//				ImGui::Image(texture ? (void*)texture->getGLid() : (void*)ui_textures[NO_TEXTURE]->getGLid(), ImVec2(64, 64));
	//				ImGui::SameLine();

	//				int w = 0; int h = 0;
	//				if (texture)
	//					texture->getSize(w, h);

	//				ImGui::Text("texture data: \n x: %d\n y: %d", w, h);

	//				if (ImGui::Button("Clear texture##clear skybox texture"))
	//					skybox->removeTexture((Direction)i);

	//				//if (ImGui::Button("Load checkered##Dif: Load checkered"))
	//				//	material->setCheckeredTexture(DIFFUSE);
	//				////ImGui::SameLine()
	//				if (ImGui::Button("Load(from asset folder)##Dif: Load"))
	//				{
	//					std::string texture_path = openFileWID();
	//					skybox->setTexture((Texture*)App->importer->ImportTexturePointer(texture_path.c_str()), (Direction)i);
	//				}
	//				ImGui::TreePop();
	//			}
	//		}
	//	}
	//}

	//ImGui::End();

}

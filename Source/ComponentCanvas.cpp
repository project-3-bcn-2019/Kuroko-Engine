#include "ComponentCanvas.h"
#include "ComponentRectTransform.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleWindow.h"
#include "ModuleTimeManager.h"
#include "GameObject.h"
#include "glew-2.1.0\include\GL\glew.h"
#include "ModuleRenderer3D.h"

#include "ImGui/imgui.h"

ComponentCanvas::ComponentCanvas(GameObject* parent) : Component(parent, CANVAS)
{
	
	rectTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);
	setResolution(float2(App->window->main_window->width, App->window->main_window->height));

	draw_cross = true;
}
ComponentCanvas::ComponentCanvas(JSON_Object * deff, GameObject * parent) : Component(parent, CANVAS) {
	   
	rectTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);

	is_active = json_object_get_boolean(deff, "active");
	draw_cross = json_object_get_boolean(deff, "debug");
	
	setResolution(float2(rectTransform->getWidth(), rectTransform->getHeight()));
}


ComponentCanvas::~ComponentCanvas()
{
}

bool ComponentCanvas::Update(float dt)
{
	return true;
}

void ComponentCanvas::Draw()
{
	App->renderer3D->orderedUI.push(this);
}

void ComponentCanvas::Render() const
{
	if (draw_cross && App->time->getGameState() != GameState::PLAYING) {
		float2 midPoint = float2(rectTransform->getWidth()/2, rectTransform->getHeight() / 2);
		glColor3f(1.0f, 0.0f, 0.0f); // red
		glLineWidth(1.5f);
		glBegin(GL_LINES);
		glVertex3f(midPoint.x, midPoint.y + 10.f, 0);
		glVertex3f(midPoint.x, midPoint.y - 10.f, 0);
		glVertex3f(midPoint.x + 10.f, midPoint.y, 0);
		glVertex3f(midPoint.x - 10.f, midPoint.y, 0);
		glEnd();
		glLineWidth(1.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
	}
}

bool ComponentCanvas::DrawInspector(int id)
{
	if (ImGui::CollapsingHeader("Canvas"))
	{
		ImGui::Text("Resolution  %.0f x %.0f", getResolution().x, getResolution().y);
		if (ImGui::Checkbox("Draw cross hair", &draw_cross))
			App->scene->AskAutoSaveScene();
	}

	return true;
}

void ComponentCanvas::Save(JSON_Object * config)
{
	json_object_set_string(config, "type", "canvas");
	json_object_set_boolean(config, "active", is_active);
	json_object_set_boolean(config, "debug", draw_cross);

}

void ComponentCanvas::setResolution(float2 resolution)
{		
	rectTransform->setWidth(resolution.x);
	rectTransform->setHeight(resolution.y);
}

inline const float2 ComponentCanvas::getResolution()
{
	return float2(rectTransform->getWidth(), rectTransform->getHeight());
}

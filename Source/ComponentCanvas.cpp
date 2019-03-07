#include "ComponentCanvas.h"
#include "ComponentRectTransform.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "GameObject.h"
#include "glew-2.1.0\include\GL\glew.h"

ComponentCanvas::ComponentCanvas(GameObject* parent) : Component(parent, CANVAS)
{
	
	rectTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);
	setResolution(float2(App->window->main_window->width, App->window->main_window->height));

	draw_cross = true;
}
ComponentCanvas::ComponentCanvas(JSON_Object * deff, GameObject * parent) : Component(parent, CANVAS) {
	   
	rectTransform = (ComponentRectTransform*)parent->getComponent(RECTTRANSFORM);
	   
	
	setResolution(float2(rectTransform->getWidth(), rectTransform->getHeight()));
}


ComponentCanvas::~ComponentCanvas()
{
}

bool ComponentCanvas::Update(float dt)
{
	return true;
}

void ComponentCanvas::Draw() const
{
	if (draw_cross) {
		float2 midPoint = float2(_resolution.x / 2, _resolution.y / 2);
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

void ComponentCanvas::Save(JSON_Object * config)
{
	json_object_set_string(config, "type", "canvas");
	json_object_set_number(config, "resolutionX", getResolution().x);
	json_object_set_number(config, "resolutionY", getResolution().y);
}

void ComponentCanvas::setResolution(float2 resolution)
{	
	_resolution = resolution;
	rectTransform->setWidth(resolution.x);
	rectTransform->setHeight(resolution.y);
}

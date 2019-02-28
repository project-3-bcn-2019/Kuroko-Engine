#pragma once

#include "Panel.h"
#include <list>

class Component;
class ComponentAnimation;
class ResourceAnimation;

class PanelAnimation :
	public Panel
{
public:

	PanelAnimation(const char* name);

	bool fillInfo();

	~PanelAnimation();

	void Draw();

public:

	ComponentAnimation* compAnimation = nullptr;
	ResourceAnimation* animation = nullptr;
	Component* selected_component = nullptr;

	std::list<Component*> par_components;

	float numFrames = 1;

private:

	ImVec2 barMovement = { 0, 0 };
	ImVec2 mouseMovement = { 0, 0 };

	float buttonPos = 0.0f;
	float offset = 0.0f;

	bool dragging = false;
	bool scrolled = false;
	bool animplay = false;
	bool animpaus = false;

	float recSize = 700;
	float zoom = 50;
	float speed = 0.0f;
	float progress = 0.0f;
	float winSize = 0.0f;

	bool new_keyframe_win = false;
	int ev_t = -1;
	int new_key_frame = 0;


};
#ifndef __PANELANIMATION_H__
#define __PANELANIMATION_H__

#include "Panel.h"
#include <list>
#include <map>

class Component;
class ComponentAnimation;
class ResourceAnimation;
class GameObject;
class BoneTransform;
class ComponentBone;

class PanelAnimation : public Panel
{
public:

	PanelAnimation(const char* name);

	bool fillInfo();

	~PanelAnimation();

	void Draw();
	void ComponentAnimationDraw();
	void TryPushKey();
public:

	GameObject* selected_obj = nullptr;

	ComponentAnimation* compAnimation = nullptr;
	ResourceAnimation* animation = nullptr;
	BoneTransform* selectedBone = nullptr;
	ComponentBone* compBone = nullptr;
	Component* selected_component = nullptr;	
	std::list<Component*> par_components;

	float numFrames = 1;

private:

	ImVec2 barMovement = { 0, 0 };
	ImVec2 mouseMovement = { 0, 0 };

	float buttonPos = 0.0f;
	float offset = 0.0f;

	int frames = 0;

	bool dragging = false;
	bool scrolled = false;
	bool animplay = false;
	bool animpaus = false;

	float recSize = 700;
	float zoom = 50;
	float speed = 0.0f;
	float progress = 0.0f;
	float winSize = 400.f;

	// Component Animation
	int ev_t = -1;
	int new_key_frame = 0;

	std::pair<int, void*> PushEvt;
	Component* sel_comp = nullptr;
	bool token_false;

public:
	void ResetNewKeyValues()
	{
		ev_t = -1;
		new_key_frame = 0;
		PushEvt.first = -1;
		sel_comp = nullptr;
	}
};

#endif // !__PANELANIMATION_H__
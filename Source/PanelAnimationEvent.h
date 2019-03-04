#ifndef __PANELANIMATIONEVENT_H__
#define __PANELANIMATIONEVENT_H__
#include "Panel.h"
#include <list>
#include "ComponentAnimationEvent.h"

class Component;
class ComponentAnimation;
class ResourceAnimation;
class GameObject;

class PanelAnimationEvent : public Panel
{
public:

	PanelAnimationEvent(const char* name);

	bool fillInfo();

	~PanelAnimationEvent();

	void Draw();

	void AddKeyframe();
	void DeleteKeyframe();

	void CopySpecs();
	
public:

	GameObject* selected_obj = nullptr;

	ComponentAnimationEvent* c_AnimEvt = nullptr;
	//ResourceAnimationEvent* r_AnimEvt = nullptr; // When Available
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

	// Component Animation
	bool new_keyframe_win = false;
	bool del_keyframe_win = false;
	int ev_t = -1;
	int new_key_frame = 0;
	int key_to_del = -1;

	

public:
	bool new_anim_set_win = false;
	char prov[50] = "\0";

	std::list<AnimSet>::iterator curr;

	void ResetNewKeyValues()
	{
		new_keyframe_win = false;
		ev_t = -1;
		new_key_frame = 0;
	}
};

#endif
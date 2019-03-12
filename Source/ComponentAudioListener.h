#ifndef _COMPONENT_AUDIO_LISTENER_H_
#define _COMPONENT_AUDIO_LISTENER_H_

#include "Component.h"
#include "Wwise.h"


class WwiseGameObject;

class ComponentAudioListener : public Component
{
public:
	ComponentAudioListener(GameObject* parent);
	ComponentAudioListener(JSON_Object* deff, GameObject* parent);
	~ComponentAudioListener();

	bool Update(float dt) override;
	void DrawInspector(int id = 0);
	void CleanUp();
	//void SetInspectorInfo() override;

	void Save(JSON_Object* config) override;

public:
	Wwise::WwiseGameObject* sound_go = nullptr;

};

#endif
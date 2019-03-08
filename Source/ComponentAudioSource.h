#ifndef _COMPONENT_AUDIO_SOURCE_H_
#define _COMPONENT_AUDIO_SOURCE_H_

#include "Component.h"
#include "Wwise.h"
//#include "Wwise_IDs.h"

class WwiseGameObject;

enum AudioAnimEvents {
	AUDIO_NONE,
	AUDIO_PLAY,
	AUDIO_STOP,
	AUDIO_PAUSE,
	AUDIO_RESUME,
	AUDIO_AMOUNT_OF_EVENTS
};

class ComponentAudioSource : public Component
{
public:
	ComponentAudioSource(GameObject* parent);
	ComponentAudioSource(JSON_Object* deff, GameObject* parent);
	~ComponentAudioSource();

	bool Update(float dt) override;
	void CleanUp();
	//void SetInspectorInfo() override;

	void Save(JSON_Object* config) override;

	void Play();
	void Stop();
	void Pause();
	void Resume();
	void SetSoundID(AkUniqueID ID);
	void SetSoundName(const char* newName);

	// Animation Events
	std::string EvTypetoString(int evt) override;
	int getEvAmount() override;
	void ProcessAnimationEvents(std::map<int, void*>& evts) override;

public:
	Wwise::WwiseGameObject* sound_go = nullptr;
	AkUniqueID sound_ID = 0;
	std::string name = "Sound";
	int volume = 100;
	int pitch = 0;
};

#endif
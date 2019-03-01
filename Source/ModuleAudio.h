#ifndef _MODULE_AUDIO_H_
#define _MODULE_AUDIO_H_

#include "Module.h"
#include "Wwise.h"


#define AUDIO_DIRECTORY "Library/Sounds/"
#define AUDIO_EXTENSION ".bnk"
#define DEFAULT_VOLUME 75


class ModuleAudio : public Module {

public:

	ModuleAudio(Application* app, bool start_enabled = true);
	virtual ~ModuleAudio();
	bool Init(const JSON_Object* config) override;
	bool Start() override;
	update_status PreUpdate(float dt) override;
	update_status PostUpdate(float dt) override;
	bool CleanUp() override;

	void SaveConfig(JSON_Object* config)const override;
	void LoadConfig(const JSON_Object* config) override;

	void LoadSoundBank(const char* path);
	
	void SetRTCP(const char* rtpc, float value, AkGameObjectID id = AK_INVALID_GAME_OBJECT);
	void SetVolume(float value = DEFAULT_VOLUME, AkGameObjectID id = AK_INVALID_GAME_OBJECT);
	void SetPitch(float value = DEFAULT_VOLUME, AkGameObjectID id = AK_INVALID_GAME_OBJECT);

	void GetBanksAndEvents();

public:
	int volume = DEFAULT_VOLUME;
	bool muted = false;

	std::vector<std::string> soundBanks;
	std::vector<std::string> events;
};


#endif // _MODULE_AUDIO_H_
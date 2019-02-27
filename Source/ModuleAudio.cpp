#include "Globals.h"
#include "Application.h"
#include "ModuleAudio.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleCamera3D.h"
#include "ModuleUi.h"
#include "ModuleImporter.h"
#include "GameObject.h"
#include "ComponentCamera.h"
#include "ComponentTransform.h"
#include "ComponentAudioListener.h"
#include "Camera.h"
#include "Wwise.h"
#include "Applog.h"

#include <corecrt_wstring.h>

#include "Wwise/IO/Win32/AkFilePackageLowLevelIOBlocking.h"
#include "../Game/Assets/Sounds/Wwise_IDs.h"


ModuleAudio::ModuleAudio(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	name = "audio";
}

ModuleAudio::~ModuleAudio()
{

}

bool ModuleAudio::Init(const JSON_Object* config)
{
	App->importer->ImportSounds();

	app_log->AddLog("Initializing Wwise");
	
	bool ret = Wwise::InitWwise();
	
	LoadSoundBank("Character");
	
	return ret;
}

bool ModuleAudio::Start()
{
	SetVolume(75);

	return true;
}

update_status ModuleAudio::PreUpdate(float dt)
{
	/*if (!muted) {
		SetVolume("Volume", volume);
	}
	else {
		SetVolume("Volume", 0);
	}*/

	// Set camera listener to camera position
	if (App->scene->audiolistenerdefault)
	{
		float3 cam_pos = App->camera->selected_camera->getFrustum()->pos;
		((ComponentTransform*)App->scene->audiolistenerdefault->getComponent(TRANSFORM))->local->setPosition(cam_pos);
		((ComponentTransform*)App->scene->audiolistenerdefault->getComponent(TRANSFORM))->local->CalculateMatrix();
	}

	return UPDATE_CONTINUE;
}


update_status ModuleAudio::PostUpdate(float dt)
{
	AK::SoundEngine::RenderAudio();

	return UPDATE_CONTINUE;
}

bool ModuleAudio::CleanUp()
{
	app_log->AddLog("Freeing Wwise");
	
	return Wwise::CloseWwise();

	return true;
}

void ModuleAudio::SaveConfig(JSON_Object* config)const
{
}

void ModuleAudio::LoadConfig(const JSON_Object* config)
{
}

// -----------------------------

void ModuleAudio::SetRTCP(const char* rtpcID, float value, AkGameObjectID id)
{
	AKRESULT eResult = AK::SoundEngine::SetRTPCValue(rtpcID, value, id);
	if (eResult != AK_Success)
		assert(!"Error setting RTPC value!");
}

void ModuleAudio::SetVolume(float value, AkGameObjectID id)
{
	AKRESULT eResult = AK_Fail;

	if (!muted)
	{
		if (id == AK_INVALID_GAME_OBJECT)
			volume = value;
		eResult = AK::SoundEngine::SetRTPCValue("VOLUME", value, id);
	}
	else
		eResult = AK::SoundEngine::SetRTPCValue("VOLUME", 0, id);

	if (eResult != AK_Success)
		assert(!"Error changing audio volume!");
}

void ModuleAudio::SetPitch(float value, AkGameObjectID id)
{
	AKRESULT eResult = AK::SoundEngine::SetRTPCValue("PITCH", value, id);
	if (eResult != AK_Success)
		assert(!"Error changing audio pitch!");
}

void ModuleAudio::LoadSoundBank(const char* path)
{
	//SoundBank* new_bank = new SoundBank();
	std::string bank_path = AUDIO_DIRECTORY;
	bank_path += path;
	bank_path += AUDIO_EXTENSION;

	Wwise::LoadBank(bank_path.c_str());

	std::string json_file = bank_path.substr(0, bank_path.find_last_of('.')) + ".json"; // Changing .bnk with .json
	/*GetBankInfo(json_file, new_bank);
	soundbanks.push_back(new_bank);
	soundbank = new_bank;
	return new_bank;*/
}
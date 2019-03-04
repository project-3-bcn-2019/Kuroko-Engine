#include "Globals.h"
#include "Application.h"
#include "ModuleAudio.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleCamera3D.h"
#include "ModuleUi.h"
#include "ModuleImporter.h"
#include "ModuleResourcesManager.h"
#include "GameObject.h"
#include "ComponentCamera.h"
#include "ComponentTransform.h"
#include "ComponentAudioListener.h"
#include "Camera.h"
#include "Wwise.h"
#include "Wwise_IDs.h"
#include "Applog.h"

#include <corecrt_wstring.h>

#include "Wwise/IO/Win32/AkFilePackageLowLevelIOBlocking.h"


ModuleAudio::ModuleAudio(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	name = "audio";
}

ModuleAudio::~ModuleAudio()
{

}

bool ModuleAudio::Init(const JSON_Object* config)
{
	//App->importer->ImportSounds();

	GetBanksAndEvents();
	
	return true;
}

bool ModuleAudio::Start()
{
	bool ret = Wwise::InitWwise();
	SetVolume(75);
	LoadSoundBank(std::to_string(App->resources->getAudioResourceUuid("Character")).c_str());

	return ret;
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
	std::string bank_path = AUDIO_FOLDER;
	bank_path += path;
	bank_path += AUDIO_EXTENSION;

	Wwise::LoadBank(bank_path.c_str());
}

void ModuleAudio::GetBanksAndEvents()
{
	if (!App->is_game)
		App->fs.copyFileTo(SOUNDBANKSINFO, LIBRARY_AUDIO, ".json", "SoundBanksInfo");

	std::vector<std::string> stringBanks;
	std::vector<std::string> stringEvents;
	std::string infoFile_path = "Library/Audio/SoundbanksInfo.json";
	JSON_Value* infoFile = json_parse_file(infoFile_path.c_str());
	if (!infoFile) {
		app_log->AddLog("Couldn't load %s, no value", infoFile_path);
		return;
	}
	JSON_Object* infoObject = json_value_get_object(infoFile);
	JSON_Object* soundBanksInfo = json_object_get_object(infoObject, "SoundBanksInfo");
	JSON_Array* banks_array = json_object_get_array(soundBanksInfo, "SoundBanks");	// Get array of soundbanks

	for (int i = 0; i < json_array_get_count(banks_array); i++) {
		JSON_Object* bank_obj = json_array_get_object(banks_array, i);
		stringBanks.push_back(json_object_get_string(bank_obj, "ShortName"));	// Get bank name

		JSON_Array* events_array = json_object_get_array(bank_obj, "IncludedEvents");	// Get array of events
		for (int j = 0; j < json_array_get_count(events_array); j++) {
			JSON_Object* event_obj = json_array_get_object(events_array, j);
			stringEvents.push_back(json_object_get_string(event_obj, "Name"));	// Get event name
		}
	}

	soundBanks = stringBanks;
	events = stringEvents;

	json_value_free(infoFile);
}
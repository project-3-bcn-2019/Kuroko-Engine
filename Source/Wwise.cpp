#include "Wwise.h"
#include "Application.h"
#include <assert.h>
#include "Globals.h"
#include "Applog.h"
#include "ModuleResourcesManager.h"
#include "Include_Wwise.h"
#include "Wwise/IO/Win32/AkFilePackageLowLevelIOBlocking.h"
#include "Wwise/IO/Win32/AkDefaultIOHookBlocking.h"
#include "Wwise/SDK/include/AkDefaultIOHookBlocking.h"
#include "Wwise/SDK/include/AkFileHelpers.h"
//#include "Wwise_IDs.h"
#include <AK/Plugin/AkRoomVerbFXFactory.h>

//CAkDefaultIOHookBlocking g_defaultIO;
CAkFilePackageLowLevelIOBlocking g_lowLevelIO;

#define BANKNAME_INIT "Init"

//Initialize all Wwise modules. Receives the base path for soundbanks and the current language
bool Wwise::InitWwise()
{
	bool ret = true;
	app_log->AddLog("Initializing Wwise");

	ret = InitMemoryManager();
	ret = InitStreamManager();
	ret = InitDeviceSettings();
	ret = InitSoundEngine();
	ret = InitMusicEngine();

#ifndef AK_OPTIMIZED
	// Initialize communications (not in release build!)
	AkCommSettings commSettings;
	AK::Comm::GetDefaultInitSettings(commSettings);
	if (AK::Comm::Init(commSettings) != AK_Success)
	{
		assert(!"Could not initialize communication.");
		ret = false;
	}
#endif // AK_OPTIMIZED

	//g_lowLevelIO.SetBasePath(AKTEXT("../../../samples/IntegrationDemo/WwiseProject/GeneratedSoundBanks/Windows/"));
	/*if (AK::StreamMgr::SetCurrentLanguage((AkOSChar*)("English")) != AK_Success)
	{
		assert(!"Error setting language!");
	}*/
	
	std::string initBank_path = AUDIO_FOLDER;
	initBank_path += std::to_string(App->resources->getAudioResourceUuid(BANKNAME_INIT));
	initBank_path += AUDIO_EXTENSION;
	LoadBank(initBank_path.c_str());

	return ret;
}

bool Wwise::InitMemoryManager()
{
	// Initializing default memory manager
	AkMemSettings memSettings;
	memSettings.uMaxNumPools = 20;

	if (AK::MemoryMgr::Init(&memSettings) != AK_Success)
	{
		assert(!"Could not create the memory manager.");
		return false;
	}

	return true;
}

bool Wwise::InitStreamManager()
{
	// Initializing the Streaming Manager
	AkStreamMgrSettings stmSettings;
	AK::StreamMgr::GetDefaultSettings(stmSettings);

	// Customize the Stream Manager settings here
	if (!AK::StreamMgr::Create(stmSettings))
	{
		assert(!"Could not create the Streaming Manager");
		return false;
	}
	// -------------------
	AkDeviceSettings deviceSettings;
	AK::StreamMgr::GetDefaultDeviceSettings(deviceSettings);
	if (g_lowLevelIO.Init(deviceSettings) != AK_Success)
	{
		assert(!"Could not create the streaming device and Low-Level I/O system");
		return false;
	}

	return true;
}

bool Wwise::InitDeviceSettings()
{
	// Initializing the default IO device
	AkDeviceSettings deviceSettings;
	AK::StreamMgr::GetDefaultDeviceSettings(deviceSettings);
	if (g_lowLevelIO.Init(deviceSettings) != AK_Success)
	{
		assert(!"Could not create the streaming device and Low-Level I/O system");
		return false;
	}

	return true;
}

bool Wwise::InitSoundEngine()
{
	// Initializing the Sound Engine
	AkInitSettings initSettings;
	//AkPlatformInitSettings platformInitSettings;
	AK::SoundEngine::GetDefaultInitSettings(initSettings);
	//AK::SoundEngine::GetDefaultPlatformInitSettings(platformInitSettings);

	if (AK::SoundEngine::Init(&initSettings, NULL) != AK_Success)
	{
		assert(!"Could not initialize the Sound Engine.");
		return false;
	}

	return true;
}

bool Wwise::InitMusicEngine()
{
	// Initializing the Music Engine
	AkMusicSettings musicInit;
	AK::MusicEngine::GetDefaultInitSettings(musicInit);

	if (AK::MusicEngine::Init(&musicInit) != AK_Success)
	{
		assert(!"Could not initialize the Music Engine.");
		return false;
	}

	return true;
}

bool Wwise::CloseWwise()
{
#ifndef AK_OPTIMIZED
	// Terminate Communication Services
	AK::Comm::Term();
#endif // AK_OPTIMIZED

	// Terminate the music engine
	AK::MusicEngine::Term();

	// Terminate the sound engine
	AK::SoundEngine::Term();

	// Terminate the streaming device and streaming manager
	g_lowLevelIO.Term();
	if (AK::IAkStreamMgr::Get())
		AK::IAkStreamMgr::Get()->Destroy();

	// Terminate the Memory Manager
	AK::MemoryMgr::Term();

	return true;
}

AkBankID Wwise::LoadBank(const char* name)
{
	AkBankID bankID;
	AKRESULT eResult = AK::SoundEngine::LoadBank(name, AK_DEFAULT_POOL_ID, bankID);
	if (eResult == AK_WrongBankVersion)
	{
		assert(!"WrongBankVersion!");
		return false;
	}
	else if (eResult != AK_Success)
	{
		assert(!"Could not load soundbank!");
		return false;
	}

	return bankID;
}

Wwise::WwiseGameObject::WwiseGameObject(unsigned long idGO, const char* nameGO)
{
	ID = idGO;
	name = nameGO;

	AKRESULT eResult = AK::SoundEngine::RegisterGameObj(ID, name);
	if (eResult != AK_Success)
	{
		app_log->AddLog("Failed to register GameObject to Wwise!");
	}
}

Wwise::WwiseGameObject::~WwiseGameObject()
{
	AKRESULT eResult = AK::SoundEngine::UnregisterGameObj(ID);
	if (eResult != AK_Success)
	{
		app_log->AddLog("Failed to unregister GameObject from Wwise!");
	}
}

Wwise::WwiseGameObject* Wwise::CreateSoundObj(unsigned long id, const char* name, float x, float y, float z, bool is_default_listener)
{
	WwiseGameObject* emitter = nullptr;

	emitter = new WwiseGameObject(id, name);

	if (is_default_listener)
	{
		AkGameObjectID listener_id = emitter->GetID();
		AK::SoundEngine::SetDefaultListeners(&listener_id, 1);
	}

	emitter->SetPosition(x, y, z);

	return emitter;
}

unsigned long Wwise::WwiseGameObject::GetID()
{
	return ID;
}

const char* Wwise::WwiseGameObject::GetName()
{
	return name;
}

float3 Wwise::WwiseGameObject::GetPos()
{
	return { position.X, position.Y, position.Z };
}

void Wwise::WwiseGameObject::SetPosition(float x, float y, float z, float x_front, float y_front, float z_front, float x_top, float y_top, float z_top)
{
	position.X = -x;
	position.Y = y;
	position.Z = -z;

	front.X = x_front;
	front.Y = y_front;
	front.Z = z_front;
	top.X = x_top;
	top.Y = y_top;
	top.Z = z_top;

	float length_front = sqrt(pow(front.X, 2) + pow(front.Y, 2) + pow(front.Z, 2));
	float length_top = sqrt(pow(top.X, 2) + pow(top.Y, 2) + pow(top.Z, 2));

	//Normalize vectors
	front.X = front.X / length_front;
	front.Y = front.Y / length_front;
	front.Z = front.Z / length_front;
	top.X = top.X / length_top;
	top.Y = top.Y / length_top;
	top.Z = top.Z / length_top;

	//Check if the are orthogonals
	float dot_prod = top.X*front.X + top.Y*front.Y + top.Z*front.Z;

	if (dot_prod >= 0.0001)
		app_log->AddLog("Vectors are not orthogonal!");

	AkSoundPosition sound_pos;
	sound_pos.Set(position, front, top);
	AKRESULT res = AK::SoundEngine::SetPosition((AkGameObjectID)ID, sound_pos);
	if (res != AK_Success)
		app_log->AddLog("Couldn't update position");
}

void Wwise::WwiseGameObject::PlayEvent(unsigned long id)
{
	if (AK::SoundEngine::PostEvent(id, ID) == AK_INVALID_PLAYING_ID)
	{
		assert(!"Error playing event");
	}
}

void Wwise::WwiseGameObject::PauseEvent(unsigned long id)
{
	if (AK::SoundEngine::ExecuteActionOnEvent(id, AK::SoundEngine::AkActionOnEventType::AkActionOnEventType_Pause, ID) == AK_INVALID_PLAYING_ID)
	{
		assert(!"Error pausing event");
	}
}

void Wwise::WwiseGameObject::ResumeEvent(unsigned long id)
{
	if (AK::SoundEngine::ExecuteActionOnEvent(id, AK::SoundEngine::AkActionOnEventType::AkActionOnEventType_Resume, ID) == AK_INVALID_PLAYING_ID)
	{
		assert(!"Error resuming event");
	}
}

void Wwise::WwiseGameObject::StopEvent(unsigned long id)
{
	if (AK::SoundEngine::ExecuteActionOnEvent(id, AK::SoundEngine::AkActionOnEventType::AkActionOnEventType_Stop, ID) == AK_INVALID_PLAYING_ID)
	{
		assert(!"Error stopping event");
	}
}

void Wwise::WwiseGameObject::SetAuxiliarySends(AkReal32 value, const char* target_bus, AkGameObjectID listener_id)
{
	AkAuxSendValue reverb;
	reverb.listenerID = listener_id;
	reverb.auxBusID = AK::SoundEngine::GetIDFromString(target_bus);
	reverb.fControlValue = value;

	AKRESULT res = AK::SoundEngine::SetGameObjectAuxSendValues(listener_id, &reverb, 2);
	if (res != AK_Success)
		assert(!"Failed to SetAuxiliarySends!");
}
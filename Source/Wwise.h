#ifndef _WWISE_H_
#define _WWISE_H_

#include "MathGeoLib/MathGeoLib.h"
#include <AK/SoundEngine/Common/AkTypes.h>

namespace Wwise
{
	class WwiseGameObject
	{
	public:
		WwiseGameObject(unsigned long idGO, const char* nameGO);
		~WwiseGameObject();
		unsigned long GetID();
		const char* GetName();
		float3 GetPos();
		void SetPosition(float x = 0, float y = 0, float z = 0, float x_front = 1, float y_front = 0, float z_front = 0, float x_top = 0, float y_top = 1, float z_top = 0);
		
		void PlayEvent(unsigned long id);
		void PauseEvent(unsigned long id, int fade_time_ms = 0);
		void ResumeEvent(unsigned long id, int fade_time_ms = 0);
		void StopEvent(unsigned long id, int fade_time_ms = 0);

		void SetAuxiliarySends(AkReal32 value, const char * target_bus, AkGameObjectID listener_id);

	private:
		const char* name = nullptr;
		AkGameObjectID ID;
		AkVector position = { 0, 0, 0 };
		AkVector front = { 0, 0, 0 };
		AkVector top = { 0, 0, 0 };
		
	};
	
	bool InitWwise();
	bool InitMemoryManager();
	bool InitStreamManager();
	bool InitSoundEngine();
	bool InitMusicEngine();
	bool CloseWwise();
	bool InitDeviceSettings();

	WwiseGameObject* CreateSoundObj(unsigned long id, const char* name, float x, float y, float z, bool is_default_listener = false);
	AkBankID LoadBank(const char* name);
}

#endif

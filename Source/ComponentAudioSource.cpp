#include "ComponentAudioSource.h"
#include "Application.h"
#include "ModuleAudio.h"
#include "ModuleTimeManager.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "Transform.h"
#include "ImGui/imgui.h"
#include "Include_Wwise.h"


ComponentAudioSource::ComponentAudioSource(GameObject* parent) : Component(parent, AUDIOSOURCE)
{
	float3 pos = ((ComponentTransform*)parent->getComponent(Component_type::TRANSFORM))->local->getPosition();
	sound_go = Wwise::CreateSoundObj(random32bits(), parent->getName().c_str(), pos.x, pos.y, pos.z);
}

ComponentAudioSource::ComponentAudioSource(JSON_Object* deff, GameObject* parent) : Component(parent, AUDIOSOURCE)
{
	float3 pos = ((ComponentTransform*)parent->getComponent(Component_type::TRANSFORM))->local->getPosition();
	sound_go = Wwise::CreateSoundObj(random32bits(), parent->getName().c_str(), pos.x, pos.y, pos.z);

	is_active = json_object_get_boolean(deff, "active");
	sound_ID = json_object_get_number(deff, "soundID");
	name = json_object_get_string(deff, "soundName");
	volume = json_object_get_number(deff, "volume");
	autoplay = json_object_get_boolean(deff, "autoplay");
	App->audio->SetVolume(volume, sound_ID);
}

ComponentAudioSource::~ComponentAudioSource()
{
	CleanUp();
}


bool ComponentAudioSource::Update(float dt)
{
	float3 pos = ((ComponentTransform*)parent->getComponent(Component_type::TRANSFORM))->local->getPosition();
	sound_go->SetPosition(pos.x, pos.y, pos.z);
	PlayOnStart();

	return true;
}

bool ComponentAudioSource::DrawInspector(int id)
{
	if (ImGui::CollapsingHeader("Audio Source"))
	{
		static bool select_audio = false;

		AkUniqueID ID = sound_ID;
		if (ID != 0)
		{
			ImGui::TextColored({ 0, 1, 0, 1 }, name.c_str());

			if (ImGui::Button("Play"))
			{
				sound_go->PlayEvent(ID);
			}
			ImGui::SameLine();
			if (ImGui::Button("Stop"))
			{
				sound_go->StopEvent(ID);
			}
			ImGui::SameLine();
			if (ImGui::Button("Pause"))
			{
				sound_go->PauseEvent(ID);
			}
			ImGui::SameLine();
			if (ImGui::Button("Resume"))
			{
				sound_go->ResumeEvent(ID);
			}
			if (ImGui::SliderInt("Volume", &volume, 0, 100))
			{
				App->audio->SetVolume(volume, sound_go->GetID());
			}
			if (ImGui::SliderInt("Pitch", &pitch, -100, 100))
			{
				App->audio->SetPitch(pitch, sound_go->GetID());
			}
			if (ImGui::Button("Change Audio Event")) select_audio = true;
			ImGui::Checkbox("Autoplay", &autoplay);
		}
		else
		{
			ImGui::TextColored({ 1, 0, 0, 1 }, "No Audio Event assigned!");
			if (ImGui::Button("Set Audio Event")) select_audio = true;
		}

		if (select_audio)
		{
			ImGui::Begin("Select Audio Event", &select_audio);
			if (ImGui::MenuItem("NONE"))
			{
				SetSoundID(0);
				SetSoundName("Sound");
				select_audio = false;
			}
			for (auto it = App->audio->events.begin(); it != App->audio->events.end(); it++) {

				if (ImGui::MenuItem((*it).c_str())) {
					SetSoundID(AK::SoundEngine::GetIDFromString((*it).c_str()));
					SetSoundName((*it).c_str());
					select_audio = false;
					break;
				}
			}
			ImGui::End();
		}
	}
	return true;
}

void ComponentAudioSource::CleanUp()
{
	sound_go->StopEvent(sound_ID);
	delete sound_go;
	sound_go = nullptr;
}

void ComponentAudioSource::Save(JSON_Object* config)
{
	json_object_set_string(config, "type", "audioSource");
	json_object_set_boolean(config, "active", is_active);
	json_object_set_number(config, "soundID", sound_ID);
	json_object_set_string(config, "soundName", name.c_str());
	json_object_set_number(config, "volume", volume);
	json_object_set_number(config, "pitch", pitch);
	json_object_set_boolean(config, "autoplay", autoplay);
}

void ComponentAudioSource::Play()
{
	sound_go->PlayEvent(sound_ID);
}

void ComponentAudioSource::Stop()
{
	sound_go->StopEvent(sound_ID);
}

void ComponentAudioSource::Pause()
{
	sound_go->PauseEvent(sound_ID);
}

void ComponentAudioSource::Resume()
{
	sound_go->ResumeEvent(sound_ID);
}

void ComponentAudioSource::SetSoundID(AkUniqueID ID)
{
	sound_ID = ID;
}

void ComponentAudioSource::SetSoundName(const char* newName)
{
	name = newName;
}

void ComponentAudioSource::PlayOnStart()
{
	if (autoplay && App->time->getJustPlayed())
		Play();
}

// --------------------------------------------------------------
// Animation events functions

std::string ComponentAudioSource::EvTypetoString(int evt)
{
	switch (evt)
	{
	case AudioAnimEvents::AUDIO_PLAY:
		return "PLAY";
	case AudioAnimEvents::AUDIO_STOP:
		return "STOP";
	case AudioAnimEvents::AUDIO_PAUSE:
		return "PAUSE";
	case AudioAnimEvents::AUDIO_RESUME:
		return "RESUME";
	}
	return "ERROR";
}

int ComponentAudioSource::getEvAmount()
{
	return AudioAnimEvents::AUDIO_AMOUNT_OF_EVENTS;
}

void ComponentAudioSource::ProcessAnimationEvents(std::map<int, void*>& evts)
{
	for(auto it_evt = evts.begin(); it_evt != evts.end(); ++it_evt)
	{
		switch(it_evt->first)
		{
		case AudioAnimEvents::AUDIO_PLAY:
			Play();
			break;
		case AudioAnimEvents::AUDIO_STOP:
			Stop();
			break;
		case AudioAnimEvents::AUDIO_PAUSE:
			Pause();
			break;
		case AudioAnimEvents::AUDIO_RESUME:
			Resume();
			break;
		default:
			break;
		}
	}
}

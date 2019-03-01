#include "ComponentAudioListener.h"
#include "Application.h"
#include "ModuleAudio.h"
#include "ModuleCamera3D.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"
#include "Camera.h"


ComponentAudioListener::ComponentAudioListener(GameObject* parent) : Component(parent, AUDIOLISTENER)
{
	float3 pos = ((ComponentTransform*)parent->getComponent(Component_type::TRANSFORM))->local->getPosition();
	sound_go = Wwise::CreateSoundObj(random32bits(), parent->getName().c_str(), pos.x, pos.y, pos.z, true);
}

ComponentAudioListener::ComponentAudioListener(JSON_Object* deff, GameObject* parent) : Component(parent, AUDIOLISTENER)
{
	App->scene->audiolistenerdefault = parent;
	float3 pos = ((ComponentTransform*)parent->getComponent(Component_type::TRANSFORM))->local->getPosition();
	sound_go = Wwise::CreateSoundObj(random32bits(), parent->getName().c_str(), pos.x, pos.y, pos.z, true);

	is_active = json_object_get_boolean(deff, "active");
	App->audio->muted = json_object_get_boolean(deff, "muted");
	App->audio->volume = json_object_get_number(deff, "volume");
	App->audio->SetVolume(json_object_get_number(deff, "volume"));
}

ComponentAudioListener::~ComponentAudioListener()
{
	CleanUp();
}


bool ComponentAudioListener::Update(float dt)
{
	float3 pos = App->camera->current_camera->getFrustum()->pos;
	float3 front = App->camera->current_camera->getFrustum()->front;
	float3 top = App->camera->current_camera->getFrustum()->up;
	sound_go->SetPosition(pos.x, pos.y, pos.z, front.x, front.y, front.z, top.x, top.y, top.z);

	return true;
}

void ComponentAudioListener::CleanUp()
{
	App->scene->audiolistenerdefault = nullptr;
	delete sound_go;
	sound_go = nullptr;
}

//void ComponentAudioListener::SetInspectorInfo()
//{
//	ImGui::Spacing();
//	if (ImGui::CollapsingHeader("Audio Listener", ImGuiTreeNodeFlags_DefaultOpen))
//	{
//		ImGui::Checkbox("Mute", &App->audio->muted);
//		if (ImGui::InputInt("Volume", &App->audio->volume))
//		{
//			App->audio->SetVolume("Volume", App->audio->volume);
//			App->audio->muted = false;
//		}
//		/*ImGui::Text("PosX %f", sound_go->GetPos().x);
//		ImGui::Text("PosY %f", sound_go->GetPos().y);
//		ImGui::Text("PosZ %f", sound_go->GetPos().z);*/
//	}
//}

void ComponentAudioListener::Save(JSON_Object* config)
{
	json_object_set_string(config, "type", "audioListener");
	json_object_set_boolean(config, "active", is_active);
	json_object_set_boolean(config, "muted", App->audio->muted);
	json_object_set_number(config, "volume", App->audio->volume);
}

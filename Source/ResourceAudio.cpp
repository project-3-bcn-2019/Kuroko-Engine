#include "ResourceAudio.h"
#include "Application.h"


ResourceAudio::ResourceAudio(resource_deff deff) : Resource(deff)
{
	audio_name = deff.asset;
	App->fs.getFileNameFromPath(audio_name);
	asset = audio_name;
}


ResourceAudio::~ResourceAudio()
{
}

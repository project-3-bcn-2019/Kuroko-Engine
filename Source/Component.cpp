#include "Component.h"

std::string Component::TypeToString()
{
	std::string uid = std::to_string(uuid);

	switch (type)
	{
	case NONE: return ("NONE : " + uid);
	case MESH: return ("MESH : " + uid);
	case TRANSFORM: return ("TRANSFORM : " + uid);
	case C_AABB: return ("AABB : " + uid);
	case CAMERA: return ("CAMERA : " + uid);
	case SCRIPT: return ("SCRIPT : " + uid);
	case BONE: return ("BONE : " + uid);
	case ANIMATION: return ("ANIMATION : " + uid);
	case CANVAS: return ("CANVAS : " + uid);
	case RECTTRANSFORM: return ("RECT TRANSFORM : " + uid);
	case UI_IMAGE: return ("UI IMAGE : " + uid);
	case UI_CHECKBOX: return ("UI CHECKBOX : " + uid);
	case UI_BUTTON: return ("UI BUTTON : " + uid);
	case UI_TEXT: return ("UI TEXT : " + uid);
	case AUDIOLISTENER: return ("AUDIO LISTENER : " + uid);
	case AUDIOSOURCE: return ("AUDIOSOURCE : " + uid);
	case COLLIDER_CUBE: return ("COLLIDER CUBE : " + uid);
	case BILLBOARD: return ("BILLBOARD : " + uid);
	case PARTICLE_EMITTER: return ("PARTICLE EMITTER : " + uid);
	case ANIMATION_EVENT: return ("ANIMATION EVENT : " + uid);
	}

	return uid;
}

void Component::SaveCompUUID(JSON_Object * config)
{
	json_object_set_number(config, "uuid", uuid);
}

void Component::LoadCompUUID(JSON_Object * deff)
{
	uuid = json_object_get_number(deff, "uuid");
	if (uuid == 0) uuid = random32bits();
}

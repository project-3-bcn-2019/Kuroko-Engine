#include "Component.h"

std::string Component::TypeToString()
{
	std::string uid = std::to_string(uuid);

	switch (type)
	{
	case NONE: return ("NONE" + uid);
	case MESH: return ("MESH" + uid);
	case TRANSFORM: return ("TRANSFORM" + uid);
	case C_AABB: return ("C_AABB" + uid);
	case CAMERA: return ("CAMERA" + uid);
	case SCRIPT: return ("SCRIPT" + uid);
	case BONE: return ("BONE" + uid);
	case ANIMATION: return ("ANIMATION" + uid);
	case CANVAS: return ("CANVAS" + uid);
	case RECTTRANSFORM: return ("RECTTRANSFORM" + uid);
	case UI_IMAGE: return ("UI_IMAGE" + uid);
	case UI_CHECKBOX: return ("UI_CHECKBOX" + uid);
	case UI_BUTTON: return ("UI_BUTTON" + uid);
	case UI_TEXT: return ("UI_TEXT" + uid);
	case AUDIOLISTENER: return ("AUDIOLISTENER" + uid);
	case AUDIOSOURCE: return ("AUDIOSOURCE" + uid);
	}
	return uid;
}
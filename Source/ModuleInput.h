#ifndef __MODULE_INPUT
#define __MODULE_INPUT
#include "Module.h"
#include "Globals.h"

#define MAX_MOUSE_BUTTONS 5

enum KEY_STATE
{
	KEY_IDLE = 0,
	KEY_DOWN,
	KEY_REPEAT,
	KEY_UP
};

enum CONTROLLER_BUTTON {
	BUTTON_INVALID = -1,
	BUTTON_A,
	BUTTON_B,
	BUTTON_X,
	BUTTON_Y,
	BUTTON_BACK,
	BUTTON_GUIDE,
	BUTTON_START,
	BUTTON_LEFTSTICK,
	BUTTON_RIGHTSTICK,
	BUTTON_LEFTSHOULDER,
	BUTTON_RIGHTSHOULDER,
	BUTTON_DPAD_UP,
	BUTTON_DPAD_DOWN,
	BUTTON_DPAD_LEFT,
	BUTTON_DPAD_RIGHT,
	AXIS_LEFTX_NEGATIVE,
	AXIS_LEFTX_POSITIVE,
	AXIS_LEFTY_NEGATIVE,
	AXIS_LEFTY_POSITIVE,
	AXIS_RIGHTX_NEGATIVE,
	AXIS_RIGHTX_POSITIVE,
	AXIS_RIGHTY_NEGATIVE,
	AXIS_RIGHTY_POSITIVE,
	AXIS_TRIGGERLEFT,
	AXIS_TRIGGERRIGHT,
	BUTTON_MAX
};

class Controller
{
	public:
	Controller(SDL_GameController* controller, SDL_Haptic* controller_haptic);
	virtual ~Controller();

	bool isPressed(CONTROLLER_BUTTON button, KEY_STATE state = KEY_REPEAT) const;

	uint id;
	SDL_GameController* controller;
	SDL_Haptic* controller_haptic;
	uint getControllerID() const { return id; }

public:
	KEY_STATE buttons[CONTROLLER_BUTTON::BUTTON_MAX];
	float axes[SDL_CONTROLLER_AXIS_MAX];
	void addInput(CONTROLLER_BUTTON input);
};

class ModuleInput : public Module
{
public:
	
	ModuleInput(Application* app, bool start_enabled = true);
	~ModuleInput();

	bool Init(const JSON_Object* config);
	update_status PreUpdate(float dt);
	bool CleanUp();

	bool getControllerButton(int id, CONTROLLER_BUTTON button, KEY_STATE state);
	bool getFirstControllerButton(CONTROLLER_BUTTON button, KEY_STATE state);
	float getControllerAxis(int id, SDL_GameControllerAxis button);
	float getFirstControllerAxis(SDL_GameControllerAxis axis);


	KEY_STATE GetKey(int id) const
	{
		return keyboard[id];
	}

	KEY_STATE GetMouseButton(int id) const
	{
		return mouse_buttons[id];
	}

	int GetMouseX() const
	{
		return mouse_x;
	}

	int GetMouseY() const
	{
		return mouse_y;
	}

	int GetMouseZ() const
	{
		return mouse_z;
	}

	int GetMouseXMotion() const
	{
		return mouse_x_motion;
	}

	int GetMouseYMotion() const
	{
		return mouse_y_motion;
	}

private:
	KEY_STATE* keyboard = nullptr;
	KEY_STATE mouse_buttons[MAX_MOUSE_BUTTONS];
	int mouse_x			= 0;
	int mouse_y			= 0;
	int mouse_z			= 0;
	int mouse_x_motion  = 0;
	int mouse_y_motion  = 0;

	std::list<Controller*> controllers;
	void handleAxes(SDL_Event e);
	float axis_tolerance = 1600;

};
#endif
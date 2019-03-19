#ifndef __ModuleWindow_H__
#define __ModuleWindow_H__

#include "Module.h"
#include "SDL/include/SDL.h"

#include <list>

class Application;

#define MIN_WINDOW_WIDTH 640.0f
#define MAX_WINDOW_WIDTH 1920.0f
#define MIN_WINDOW_HEIGHT 480.0f
#define MAX_WINDOW_HEIGHT 1080.0f

struct Window
{
	SDL_Window* window = nullptr;
	SDL_Surface* screen_surface = nullptr;

	int width = 0; int height = 0; int adaptable_offsetx = 0; int adaptable_offsety = 0;
	float brightness = 1.0f;
	bool resizable = false; bool fullscreen = false; bool borderless = false; bool fulldesk = false; bool adapt_screen = false;
	uint id = 0;
};


class ModuleWindow : public Module
{
public:

	ModuleWindow(Application* app, bool start_enabled = true);

	// Destructor
	virtual ~ModuleWindow();

	bool Init(const JSON_Object* config);
	update_status Update(float dt);
	bool CleanUp();

	// id = 0 assigns to main window
	void SetTitle(const char* title, uint id = 0);
	void setResizable(bool resize, uint id = 0);
	void setFullscreen(bool fullscreen, uint id = 0);
	void setBorderless(bool borderless, uint id = 0);
	void setFullDesktop(bool fulldesk, uint id = 0);
	void setBrightness(float brightness, uint id = 0);
	void setSize(int x, int y, uint id = 0);

	void fillWindowConfig(const JSON_Object* config);

	void SaveConfig(JSON_Object* config) const;
	void LoadConfig(const JSON_Object* config);

public:

	Window* main_window = nullptr;
	std::list<Window*> aux_windows;
};

#endif // __ModuleWindow_H__
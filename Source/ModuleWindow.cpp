#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "Applog.h"

ModuleWindow::ModuleWindow(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	name = "window";
	main_window = new Window();
}

// Destructor
ModuleWindow::~ModuleWindow()
{
}

// Called before render is available
bool ModuleWindow::Init(const JSON_Object* config)
{
	app_log->AddLog("Init SDL window & surface");
	bool ret = true;

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		app_log->AddLog("SDL_VIDEO could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		fillWindowConfig(config);
		//Create window
		int width = main_window->width * SCREEN_SIZE; 
		int height = main_window->height * SCREEN_SIZE;
		Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

		//Use OpenGL 2.1
		//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

		if(main_window->fullscreen) flags |= SDL_WINDOW_FULLSCREEN;
		if(main_window->resizable)	flags |= SDL_WINDOW_RESIZABLE;
		if(main_window->borderless) flags |= SDL_WINDOW_BORDERLESS;
		if(main_window->fulldesk)	flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

		if(main_window->adapt_screen) {
		SDL_DisplayMode DM;
		SDL_GetCurrentDisplayMode(0, &DM);
		main_window->width = width = DM.w - main_window->adaptable_offsetx;
		main_window->height = height = DM.h - main_window->adaptable_offsety;
		}
		
		main_window->window = SDL_CreateWindow(((App->is_game)?  App->game_title.c_str():App->engine_title.c_str()), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);

		if(main_window->window == NULL)
		{
			app_log->AddLog("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			ret = false;
		}
		else
		{
			//Get window surface
			main_window->screen_surface = SDL_GetWindowSurface(main_window->window);
		}
	}
	setBrightness(main_window->brightness);

	return ret;
}

update_status ModuleWindow::Update(float dt) {
	
	int window_x, window_y;
	window_x = window_y = 0;
	SDL_GetWindowSize(main_window->window, &window_x, &window_y);

	main_window->height = window_y;
	main_window->width = window_x;
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleWindow::CleanUp()
{
	app_log->AddLog("Destroying SDL window and quitting all SDL systems");

	//Destroy window
	if(main_window->window)
		SDL_DestroyWindow(main_window->window);

	for (std::list<Window*>::iterator it = aux_windows.begin(); it != aux_windows.end(); it++)
	{
		if((*it)->window)
			SDL_DestroyWindow(main_window->window);
	}

	delete main_window;

	for (auto it = aux_windows.begin(); it != aux_windows.end(); it++)
		delete *it;

	aux_windows.clear();

	//Quit SDL subsystems
	SDL_Quit();
	return true;
}

void ModuleWindow::SetTitle(const char* title, uint id)
{
	if(id == 0)
		SDL_SetWindowTitle(main_window->window, title);
	else
	{
		int i = 0;
		for (std::list<Window*>::iterator it = aux_windows.begin(); it != aux_windows.end(); it++)
		{
			i++;
			if((*it)->id == i)
				SDL_SetWindowTitle((*it)->window, title);
			break;
		}
	}
}

void ModuleWindow::setResizable(bool resizable, uint id) 
{
	if (id == 0)
		SDL_SetWindowResizable(main_window->window, (SDL_bool)resizable);
	else
	{
		int i = 0;
		for (std::list<Window*>::iterator it = aux_windows.begin(); it != aux_windows.end(); it++)
		{
			i++;
			if ((*it)->id == i)
				SDL_SetWindowResizable((*it)->window, (SDL_bool)resizable);
			break;
		}
	}
}

void ModuleWindow::setFullscreen(bool fullscreen, uint id) {

	Uint32 flag = fullscreen ? SDL_WINDOW_FULLSCREEN : 0;

	if (id == 0)
		SDL_SetWindowFullscreen(main_window->window, flag);
	else
	{
		int i = 0;
		for (std::list<Window*>::iterator it = aux_windows.begin(); it != aux_windows.end(); it++)
		{
			i++;
			if ((*it)->id == i)
				SDL_SetWindowFullscreen((*it)->window, flag);
			break;
		}
	}
}

void ModuleWindow::setBorderless(bool borderless, uint id) 
{
	if (id == 0)
		SDL_SetWindowBordered(main_window->window, (SDL_bool)!borderless);
	else
	{
		int i = 0;
		for (std::list<Window*>::iterator it = aux_windows.begin(); it != aux_windows.end(); it++)
		{
			i++;
			if ((*it)->id == i)
				SDL_SetWindowBordered((*it)->window, (SDL_bool)!borderless);
			break;
		}
	}

}

void ModuleWindow::setFullDesktop(bool fulldesktop, uint id) {
	Uint32 flag = fulldesktop ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;

	if (id == 0)
		SDL_SetWindowFullscreen(main_window->window, flag);
	else
	{
		int i = 0;
		for (std::list<Window*>::iterator it = aux_windows.begin(); it != aux_windows.end(); it++)
		{
			i++;
			if ((*it)->id == i)
				SDL_SetWindowFullscreen((*it)->window, flag);
			break;
		}
	}

}

void ModuleWindow::setBrightness(float brightness, uint id) 
{
	if (id == 0)
		SDL_SetWindowBrightness(main_window->window, brightness);
	else
	{
		int i = 0;
		for (std::list<Window*>::iterator it = aux_windows.begin(); it != aux_windows.end(); it++)
		{
			i++;
			if ((*it)->id == i)
				SDL_SetWindowBrightness((*it)->window, brightness);
			break;
		}
	}

}

void ModuleWindow::setSize(int x, int y, uint id) 
{
	if (id == 0)
		SDL_SetWindowSize(main_window->window, x, y);
	else
	{
		int i = 0;
		for (std::list<Window*>::iterator it = aux_windows.begin(); it != aux_windows.end(); it++)
		{
			i++;
			if ((*it)->id == i)
				SDL_SetWindowSize((*it)->window, x, y);
			break;
		}
	}
}

void ModuleWindow::fillWindowConfig(const JSON_Object* config) 
{
	main_window->borderless = json_object_get_boolean(config, "borderless");
	main_window->fullscreen = json_object_get_boolean(config, "fullscreen");
	main_window->resizable = json_object_get_boolean(config, "resizable");
	main_window->fulldesk = json_object_get_boolean(config, "fulldesktop");
	main_window->adapt_screen = json_object_get_boolean(config, "adapt_screen");

	main_window->width = json_object_get_number(config, "width");
	main_window->height = json_object_get_number(config, "height");
	main_window->adaptable_offsetx = json_object_get_number(config, "adaptable_offsetx");
	main_window->adaptable_offsety = json_object_get_number(config, "adaptable_offsety");

	main_window->brightness = json_object_get_number(config, "brightness");
}

void ModuleWindow::SaveConfig(JSON_Object* config) const
{
	json_object_set_boolean(config, "borderless", main_window->borderless);
	json_object_set_boolean(config, "fullscreen", main_window->fullscreen);
	json_object_set_boolean(config, "resizable", main_window->resizable);
	json_object_set_boolean(config, "fulldesktop", main_window->fulldesk);
	json_object_set_boolean(config, "adapt_screen", main_window->adapt_screen);

	json_object_set_number(config, "width", main_window->width);
	json_object_set_number(config, "height", main_window->height);
	json_object_set_number(config, "adaptable_offsetx", main_window->adaptable_offsetx);
	json_object_set_number(config, "adaptable_offsety", main_window->adaptable_offsety);
	json_object_set_number(config, "brightness", main_window->brightness);
}

void ModuleWindow::LoadConfig(const JSON_Object* config) {

	fillWindowConfig(config);
	setFullscreen(json_object_get_boolean(config, "fullscreen"));
	setResizable(json_object_get_boolean(config, "resizable"));
	setFullDesktop(json_object_get_boolean(config, "fulldesktop"));
	setBorderless(json_object_get_boolean(config, "borderless"));
	int width, height = 0;
	if (!main_window->adapt_screen){
		width = json_object_get_number(config, "width");
		height = json_object_get_number(config, "height");
	}
	else {
		SDL_DisplayMode DM;
		SDL_GetCurrentDisplayMode(0, &DM);
		width = DM.w - json_object_get_number(config, "adaptable_offsetx");
		height = DM.h - json_object_get_number(config, "adaptable_offsety");

	}
	setSize(width, height);

	setBrightness(json_object_get_number(config, "brightness"));
}

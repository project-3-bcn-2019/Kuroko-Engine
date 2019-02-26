#include "ModuleTimeManager.h"
#include "Applog.h"
#include "ModuleScene.h"
#include "Application.h"
#include "ModuleScripting.h"


ModuleTimeManager::ModuleTimeManager(Application * app, bool start_enabled): Module(app, start_enabled)
{
	name = "time_manager";
	
}



ModuleTimeManager::~ModuleTimeManager()
{

}

bool ModuleTimeManager::Init(const JSON_Object * config)
{
	// Set clock configuration in some way?
	game_time.Reset();
	game_time.setScale(1);
	real_time.Reset();

	return true;
}

void ModuleTimeManager::ManageGamePaused() {
	bool state_changed = false;

	switch (last_game_state) {
	case PLAYING:
		game_time.Pause();
		state_changed = true;
		break;
	case STOPPED:
		app_log->AddLog("Can't pause game while stopped!");
		game_state = STOPPED;
		return;
		break;
	}

	frame_count++;
	if (state_changed)
		last_game_state = game_state;
}

void ModuleTimeManager::ManageGameStopped() {
	bool state_changed = false;

	switch (last_game_state) {
	case PLAYING:
	case PAUSED:
		real_time.Reset();
		game_time.Reset();
		App->scene->AskLocalLoadScene();
		frame_count = 0;
		state_changed = true;
		break;
	}

	if (state_changed)
		last_game_state = game_state;


}

void ModuleTimeManager::ManageGamePlaying() {

	bool state_changed = false;

	switch (last_game_state) {
	case PAUSED:
		game_time.Resume();
		state_changed = true;
		break;
	case STOPPED:
		real_time.Start();
		game_time.Start();
		App->scene->AskLocalSaveScene();
		state_changed = true;
		break;
	}

	frame_count++;
	if (state_changed)
		last_game_state = game_state;

}

void ModuleTimeManager::Advance(int frames) {
	if (game_state != PAUSED)
		app_log->AddLog("Can't advance frames while not paused!");
	else {
		advance = true;
		advance_frames = frames;
		advanced_frames = 0;
	}
}

float ModuleTimeManager::getDeltaTime() {
	float ret = 0;
	if (game_state != STOPPED)
		ret = real_dt;

	return ret;
}

void ModuleTimeManager::ManageAdvance() {
	if (advance) {
		game_time.Resume();
		advance = false;
		advancing = true;
	}

	if (advancing) {
		if (advance_frames > advanced_frames)
			advanced_frames++;
		else {
			game_time.Pause();
			advancing = false;
		}
	}
}

void ModuleTimeManager::Play()  { game_state = GameState::PLAYING; App->scripting->StartInstances(); };
void ModuleTimeManager::Pause() { game_state = GameState::PAUSED; App->scripting->StopInstances(); };
void ModuleTimeManager::Stop()  { game_state = GameState::STOPPED; App->scripting->PauseInstances(); };

update_status ModuleTimeManager::Update(float dt)
{
	switch (game_state) {
	case GameState::PLAYING:
		ManageGamePlaying();
		break;
	case GameState::PAUSED:
		ManageGamePaused();
		break;
	case GameState::STOPPED:
		ManageGameStopped();
		break;
	}
	ManageAdvance();
	game_time.Update(); // Clock allways updates, but can get 

	real_dt = delta_timer.Read();
	delta_timer.Start();

	return UPDATE_CONTINUE;
}

bool ModuleTimeManager::CleanUp()
{

	return true;
}



// Game Timer
void GameTimer::Start() {
	paused = false;
	last_frame_time = SDL_GetTicks();
	time = 0;
}

void GameTimer::Update() {
	if (!paused) {
		delta_time = (SDL_GetTicks() - last_frame_time)* time_scale;
		time += delta_time;
	}
	else
		delta_time = 0;

	last_frame_time = SDL_GetTicks();
}

void GameTimer::Pause() {
	paused = true;
}

void GameTimer::Resume() {
	paused = false;
}

void GameTimer::Reset() {
	time = 0;
	paused = true;
}

float GameTimer::Read() {
	return time;
}

void GameTimer::setScale(float scale) {
	time_scale = scale;
}

float GameTimer::getScale() {
	return time_scale;
}

float GameTimer::getDeltaTime() {
	return delta_time;
}

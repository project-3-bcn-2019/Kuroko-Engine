#ifndef __MODULE_TIME_MANAGER
#define __MODULE_TIME_MANAGER

#include "Module.h"
#include "Timer.h"

enum GameState { STOPPED, PAUSED, PLAYING };


class GameTimer {


public:
	GameTimer() {};
	void Start();
	void Update(); // Allways update, time is managed through pause/resume
	void Pause();
	void Resume();
	void Reset();
	float Read();
	void setScale(float scale);
	float getScale();
	float getDeltaTime();


private:
	// All in ms
	float time = 0;
	float time_scale = 1;
	float last_frame_time = 0;
	float delta_time = 0;

	bool paused = true;

};

class ModuleTimeManager: public Module
{
public:
	ModuleTimeManager(Application* app, bool start_enabled = true);
	~ModuleTimeManager();

	bool Init(const JSON_Object* config);
	update_status Update(float dt);
	bool CleanUp();

	void Play();
	void Pause();
	void Stop();

	GameState getGameState() { return game_state; }

	void Advance(int frames);
	float getRealTime() { return real_time.Read();}
	float getGameTime() { return game_time.Read();}
	float getDeltaTime(); // Will only return delta if the game isn't stopped
	float getGameDeltaTime() { return game_time.getDeltaTime(); }
	int getFrameCount() { return frame_count; }
	void setTimeScale(float scale) { game_time.setScale(scale); }

private:


	void ManageGamePlaying();
	void ManageGamePaused();
	void ManageGameStopped();

	void ManageAdvance();
	GameState game_state = GameState::STOPPED;
	GameState last_game_state = GameState::STOPPED;
	float time_scale;

	int frame_count = 0;
	Timer real_time;
	Timer delta_timer;
	GameTimer game_time;

	float real_dt = 0;

	bool advance = false;
	bool advancing = false;
	int advance_frames = 0;
	int advanced_frames = 0;
};


#endif
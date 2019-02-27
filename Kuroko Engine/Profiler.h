#pragma once

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_sdl.h"
#include "ImGui/imgui_impl_opengl2.h"
#include "Timer.h"

#include <array>
#include <vector>
#include <list>
#include <map>

//#define PROFILE_TILL_END(name) prof->AskScope(name);

#define AVG_LENGTH 50

// Provide the types you want here
enum LogType
{
	LogError = -1,

	SUCCESS,	// Everything is cool
	HINT,		// Hints for debugging
	TIMING,		// Timing
	WARNING,	// Smth wrong but can continue
	CRITICAL,	// Execution should stop

	LogMAX,
};

struct KLog {

	int line = -1;
	LogType lt = LogError;
	char file[102];
	char buf[914];

	KLog() {
		for (int i = 0; i < 914; i++)
			buf[i] = '\0';
		for (int i = 0; i < 102; i++)
			file[i] = '\0';
	}
};

struct ProfileScope {
	std::vector<ProfileScope*> scopes;
	std::string name;
	std::vector<KLog*> logs_in_scope;
	Timer timer;
	std::vector<uint>	frames;		// Frames the results correspond to
	std::vector<float>	results;	// Results of timing
	Uint32 peak;

	ProfileScope* parent = nullptr;

	ProfileScope();
	ProfileScope(const char* str, ProfileScope* par);
	~ProfileScope();

	ProfileScope* Find(const char* name);
	void Close();
};

struct Profiler {
	Profiler();

	// Logs
	std::vector<KLog>	logs;
	ImGuiTextFilter     Filter;
	bool                ScrollToBottom;

	void	Clear();
	void	AddLog(const char file[], int line, LogType lt, const char* format, ...) IM_FMTARGS(2);
	void	LogDraw(const char* title, bool* p_open = NULL);

	// Profiling
	ProfileScope	base;
	ProfileScope*	last_open_scope;
	ProfileScope*	draw_curr;
	uint	frames;

	void	AskScope(const char* name);
	void	CloseLastScope();
	void	DrawScope(const char* title, bool* p_open = NULL);

	void	StartFrame();
	void	CloseFrame();

	// Profiling Controls
	bool paused;
	int starting_frame, ending_frame;
	int stopped_sec;

};

extern Profiler* prof;

#define PROFILE_SCOPE_START(name) prof->AskScope(name); // On Error you are lacking a PROFILE_SCOPE_END
#define PROFILE_SCOPE_END prof->CloseLastScope(); // Ends the last profiling scope you started

#define APP_CYCLE_START prof->StartFrame();
#define APP_CYCLE_END prof->CloseFrame();
#pragma once

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_sdl.h"
#include "ImGui/imgui_impl_opengl2.h"

#include <array>
#include <vector>

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

struct Profiler {
	std::vector<KLog>	logs;
	ImGuiTextFilter     Filter;
	bool                ScrollToBottom;

	void	Clear();
	void	AddLog(const char file[], int line, LogType lt, const char* format, ...) IM_FMTARGS(2);
	void	LogDraw(const char* title, bool* p_open = NULL);
};
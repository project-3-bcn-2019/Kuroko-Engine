// ----------------------------------------------------
// Timer.cpp
// Body for CPU Tick Timer class
// ----------------------------------------------------

#include "Timer.h"

// ---------------------------------------------
Timer::Timer()
{
	Start();
}

// ---------------------------------------------
void Timer::Start()
{
	reset = false;
	running = true;
	started_at = SDL_GetTicks();
	time_paused = started_at;
	stopped_at = 0;
}

// ---------------------------------------------
void Timer::Reset()
{
	reset = true;
}

void Timer::Pause() {
	stopped_at = SDL_GetTicks();
	running = false;
}

void Timer::Resume() {
	time_paused += (SDL_GetTicks() - stopped_at);
	running = true;
}

// ---------------------------------------------
Uint32 Timer::Read() const
{
	if (!reset)
	{
		if (running)
			return SDL_GetTicks() - time_paused;
		else
			return stopped_at;
	}
	else
		return 0;
}

//==============================================================
//==============================================================

PerfTimer::PerfTimer() : /*is_stopped(false),*/ started_at(std::chrono::steady_clock::now())
{
}

Uint32 PerfTimer::ReadMicroS()
{
	/*if (is_stopped)
		return std::chrono::duration_cast<std::chrono::microseconds>(stopped_at - started_at).count();*/
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - started_at).count();
}

Uint32 PerfTimer::ReadMiliS()
{
	/*if (is_stopped)
		return std::chrono::duration_cast<std::chrono::milliseconds>(stopped_at - started_at).count();*/
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - started_at).count();
}

void PerfTimer::Start()
{
	started_at = std::chrono::steady_clock::now();
}

/*
void PerfTimer::Stop()
{
	stopped_at = std::chrono::steady_clock::now();
	is_stopped = true;
}

void PerfTimer::Resum()
{
	if (is_stopped)
	{
		started_at = (std::chrono::steady_clock::now() - stopped_at - started_at);
		is_stopped = false;
	}
}*/
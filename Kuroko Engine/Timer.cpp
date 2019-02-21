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

STDLTimer::STDLTimer() : /*is_stopped(false),*/ started_at(std::chrono::steady_clock::now())
{
}

Uint32 STDLTimer::ReadMicroS()
{
	/*if (is_stopped)
		return std::chrono::duration_cast<std::chrono::microseconds>(stopped_at - started_at).count();*/
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - started_at).count();
}

Uint32 STDLTimer::ReadMiliS()
{
	/*if (is_stopped)
		return std::chrono::duration_cast<std::chrono::milliseconds>(stopped_at - started_at).count();*/
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - started_at).count();
}

void STDLTimer::Start()
{
	started_at = std::chrono::steady_clock::now();
}

//==============================================================
//==============================================================

PerfTimer::PerfTimer()
{
	timer.start();
	started_at = timer.elapsed().wall;
	stopped = false;
}

PerfTimer::~PerfTimer()
{
	// Log out the time
}

bool PerfTimer::IsStopped() const
{
	return stopped;
}

void PerfTimer::Start()
{
	timer.start();
}

void PerfTimer::Stop()
{
	stopped_at = timer.elapsed().wall;
	stopped = true;
}

void PerfTimer::Resume()
{
	if (stopped)
	{
		started_at = (Uint32)(timer.elapsed().wall - stopped_at - started_at);
		stopped = false;
	}
}

Uint32 PerfTimer::Read()
{
	
	if (!stopped)
		return SDL_GetTicks() - (started_at / T1000 / T1000);
	else
		return stopped_at;
}

Uint32 PerfTimer::ReadMicroS()
{
	if(stopped)
		return (Uint32)((stopped_at - started_at)/T1000);
	return (Uint32)((timer.elapsed().wall - started_at)/T1000);
}

Uint64 PerfTimer::ReadNanoS()
{
	if (stopped)
		return (Uint32)(stopped_at - started_at);
	return (Uint32)(timer.elapsed().wall - started_at);
}
// ----------------------------------------------------
// Timer.cpp
// Body for CPU Tick Timer class
// ----------------------------------------------------

#include "Timer.h"

// ----------------------------------------------------
Timer::Timer() : stopped(false), reset(false)
{
	timer.start();
	started_at = timer.elapsed().wall;
}
// ----------------------------------------------------
Timer::~Timer()
{
	// Log out the time
}
// ----------------------------------------------------
bool Timer::IsStopped() const
{
	return stopped;
}
// ----------------------------------------------------
void Timer::Start()
{
	started_at = timer.elapsed().wall;
	ticks_started_at = SDL_GetTicks();
}
// ----------------------------------------------------
void Timer::Pause()
{
	stopped_at = timer.elapsed().wall;
	stopped = true;
}
// ----------------------------------------------------
void Timer::Resume()
{
	if (stopped)
	{
		started_at = (Uint32)(timer.elapsed().wall - stopped_at - started_at);
		stopped = false;
	}
}
// ----------------------------------------------------
void Timer::Reset()
{
	reset = true;
}
// ----------------------------------------------------
Uint32 Timer::Read()
{
	if (reset)
		return 0;

	Uint32 val = SDL_GetTicks();

	if (!stopped)
		return SDL_GetTicks() - ticks_started_at;
	else
		return stopped_at;
}
// ----------------------------------------------------
Uint32 Timer::ReadMicroS()
{
	if (reset)
		return 0;

	if(stopped)
		return (Uint32)((stopped_at - started_at)/T1000);
	return (Uint32)((timer.elapsed().wall - started_at)/T1000);
}
// ----------------------------------------------------
Uint64 Timer::ReadNanoS()
{
	if (reset)
		return 0;

	if (stopped)
		return (Uint32)(stopped_at - started_at);
	return (Uint32)(timer.elapsed().wall - started_at);
}
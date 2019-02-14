#ifndef __TIMER_H__
#define __TIMER_H__

#include "Globals.h"
#include "SDL\include\SDL.h"
#include <ctime>
#include <chrono>

#define MILISEC (CLOCKS_PER_SEC / (clock_t)1000)
#define MICROSEC (CLOCKS_PER_SEC / (clock_t)1000000)
class Timer
{
public:

	// Constructor
	Timer();

	void Start();
	void Reset();
	void Pause();
	void Resume();

	Uint32 Read() const;

private:

	bool	running = false;
	bool	reset = false;
	Uint32	started_at;
	Uint32	stopped_at;
	Uint32 time_paused;
};

class PerfTimer
{
public:

	PerfTimer();
	virtual ~PerfTimer() {};

	void Start();
	/*void Stop();
	void Resum();*/

	Uint32 ReadMiliS();
	Uint32 ReadMicroS();

private:
	std::chrono::time_point<std::chrono::steady_clock> started_at;
	//std::chrono::time_point<std::chrono::steady_clock> stopped_at;

	//bool is_stopped;
};

#endif //__TIMER_H__
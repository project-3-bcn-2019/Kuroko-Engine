#ifndef __TIMER_H__
#define __TIMER_H__

#include "Globals.h"
#include "SDL\include\SDL.h"
#include <ctime>
#include <chrono>
//#include <boost/timer.hpp>
#include <boost/timer/timer.hpp>
//#include <boost/chrono.hpp>
#define MILISEC (CLOCKS_PER_SEC / (clock_t)1000)
#define MICROSEC (CLOCKS_PER_SEC / (clock_t)1000000)
#define T1000 (int_least64_t) 1000
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

//==============================================================
//==============================================================

class STDLTimer
{
public:

	STDLTimer();
	virtual ~STDLTimer() {};

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

//==============================================================
//==============================================================

class PerfTimer
{
public:
	PerfTimer();
	virtual ~PerfTimer();

	void Start();
	void Stop();
	void Resume();

	Uint32 MilliS();
	Uint32 MicroS();

	bool IsStopped() const;

private:
	boost::timer::cpu_timer timer;
	int_least64_t started_at;
	int_least64_t stopped_at;
	bool stopped;
};
#endif //__TIMER_H__
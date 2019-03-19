#ifndef __TIMER_H__
#define __TIMER_H__

#include "Globals.h"
#include "SDL\include\SDL.h"
#include "Boost/include/boost/timer/timer.hpp"
#pragma comment (lib, "Boost/libx86/libboost_timer-vc141-mt-gd-x32-1_69.lib")
#pragma comment (lib, "Boost/libx86/libboost_timer-vc141-mt-x32-1_69.lib")
#define T1000 (int_least64_t)1000

class Timer
{
public:
	Timer();
	virtual ~Timer();

	void Start();
	void Pause();
	void Resume();
	void Reset();

	Uint32 Read(); // Millisecond Precision
	Uint32 ReadMicroS(); // Microsecond Precision
	Uint64 ReadNanoS(); // Nanosecond Precision (not quite as windows actually has .5 Microsecond precision as far as I know, but for idiots that want nanoseconds)

	bool IsStopped() const;

private:
	boost::timer::cpu_timer timer;
	int_least64_t started_at;
	int_least64_t stopped_at;
	bool stopped;
	bool reset;
	Uint32 ticks_started_at;
};


#endif //__TIMER_H__
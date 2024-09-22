#include "Timer.h"

Timer::Timer()
{
	start_ticks = 0;
	paused_ticks = 0;

	paused = false;
	started = false;
}

Timer::~Timer()
{
}

void Timer::start()
{
	started = true;
	paused = false;

	start_ticks = SDL_GetTicks();
	paused_ticks = 0;
}

void Timer::stop()
{
	started = false;
	paused = true;

	start_ticks = 0;
	paused_ticks = 0;
}

void Timer::pause()
{
	if (started && !paused)
	{
		paused = true;

		paused_ticks = SDL_GetTicks() - start_ticks;
		start_ticks = 0;
	}
}

void Timer::unpause()
{
	if (started && paused)
	{
		paused = false;

		start_ticks = SDL_GetTicks() - paused_ticks;
		paused_ticks = 0;
	}
}

uint32_t Timer::getTicks()
{
	uint32_t time = 0;
	if (started)
	{
		if (paused) time = paused_ticks;
		else time = SDL_GetTicks() - start_ticks;
	}
	return time;
}

bool Timer::isStarted()
{
	return started;
}

bool Timer::isPaused()
{
	return paused;
}

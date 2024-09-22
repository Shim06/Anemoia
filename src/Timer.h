#pragma once

#include <cstdint>
#include <SDL.h>

class Timer
{
public:
	Timer();
	~Timer();

	void start();
	void stop();
	void pause();
	void unpause();

	uint32_t getTicks();

    bool isStarted();
    bool isPaused();

private:
    uint32_t start_ticks;
    uint32_t paused_ticks;

    bool paused;
    bool started;
};


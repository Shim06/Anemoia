#pragma once

#include <chrono>
#include <cmath>
#include <stdint.h>
#include <iostream>
#include <thread>
#include <fstream>
#include <memory>
#include <mutex>
#include "SDL.h"
#include "Bus.h"
#include "cpu6502.h"
#include <string>
#include <thread>
#include <Windows.h>

#include "Timer.h"

class Emulator
{
public:
	typedef enum
	{
		A = (1 << 7),
		B = (1 << 6),
		Select = (1 << 5),
		Start = (1 << 4),
		Up = (1 << 3),
		Down = (1 << 2),
		Left = (1 << 1),
		Right = (1 << 0)
	} controllerControls;

	// Nes screen size
	const int SCREEN_WIDTH = 256;
	const int SCREEN_HEIGHT = 240;
	uint8_t controller = 0x00;
	uint8_t controller2 = 0x00;
	uint8_t* ptr_frame_buffer = nullptr;

	int start(const std::string& file_name, SDL_Renderer*& renderer);
	void stop();
	void reset();
	void pause();
	void unpause();
	void emulate();
	void fastForward();
	void normalSpeed();
	void toggleMute();
	SDL_Texture* getFrame();

	void setVolume(uint8_t volume);
	void setPulse1Volume(uint8_t volume);
	void setPulse2Volume(uint8_t volume);
	void setTriangleVolume(uint8_t volume);
	void setNoiseVolume(uint8_t volume);
	void setDmcVolume(uint8_t volume);

	bool isRunning();

	void setController1State(controllerControls bit, bool state);

	void loadSave(const std::string& path);
	void dumpSave(const std::string& path);

	int getElapsedTime();
	double getAvgFPS();
	SDL_Texture* frame = nullptr;


	Bus nes;
	

private:
	bool fast_forward = false;
	std::mutex mtx;

	bool running = false;
	Timer fps_timer;

	uint8_t audio_buffer_count = 0;
	uint8_t audio_buffer_skip = 0;
	uint32_t elapsed_frames = 0;
	float avg_FPS = 0;
	double fps = 60.09;
	double new_frame_time = 0;
	double frame_delay = 1000.0 / (double)fps;
	bool fullscreen = false;
	bool paused = false;
	bool reset_toggle = false;
	bool buffer_used = false;

	std::chrono::steady_clock::time_point start_time, end_time;
	std::chrono::duration<double, std::milli> elapsed_time;

	bool init_audio();
	void renderFramebuffer(SDL_Renderer*& renderer, SDL_Texture*& frame, uint8_t *framebuffer);

	std::shared_ptr<Cartridge> cart;

	SDL_AudioSpec want, have;
	SDL_AudioDeviceID dev;

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
};
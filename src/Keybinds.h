#pragma once
#include <SDL.h>
#include <string>
#include <unordered_map>

enum keys
{
	KEY_A,
	KEY_B,
	KEY_SELECT,
	KEY_START,
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_FAST_FORWARD,
	KEY_TOGGLE_FAST_FORWARD,
	KEY_SCREENSHOT,
	KEY_TOGGLE_UI,
	KEY_PAUSE,
	KEY_MUTE,
	KEY_TOGGLE_FULLSCREEN,
	KEY_RESET,
	KEY_QUICK_SAVE_STATE,
	KEY_QUICK_LOAD_STATE
};

struct controls
{
	int A = 0;
	int B = 0;
	int Select = 0;
	int Start = 0;
	int Up = 0;
	int Down = 0;
	int Left = 0;
	int Right = 0;
};

struct hotkeys
{
	int fast_forward = 0;
	int toggle_fast_forward = 0;
	int reset = 0;
	int toggle_fullscreen = 0;
	int screenshot = 0;
	int toggle_UI = 0;
	int pause = 0;
	int mute = 0;
	int quick_save_state = 0;
	int quick_load_state = 0;
};

extern std::unordered_map<SDL_KeyCode, std::string> keyboard_keybinds;
extern std::unordered_map<SDL_GameControllerButton, std::string> controller_keybinds;
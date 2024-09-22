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
	KEY_SHOW_UI,
	KEY_PAUSE,
	KEY_MUTE,
	KEY_TOGGLE_FULLSCREEN,
	KEY_RESET
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
	int show_UI = 0;
	int pause = 0;
	int mute = 0;
};

std::unordered_map<SDL_KeyCode, std::string> keyboard_keybinds =
{
	{ SDLK_RETURN, "Return" },
	{ SDLK_BACKSPACE, "Backspace" },
	{ SDLK_TAB, "Tab" },
	{ SDLK_SPACE, "Space" },
	{ SDLK_PRINTSCREEN, "PrtSc" },
	{ SDLK_SCROLLLOCK, "ScrollLock" },
	{ SDLK_CAPSLOCK, "CapsLock" },
	{ SDLK_PAUSE, "Pause" },
	{ SDLK_DELETE, "Delete" },
	{ SDLK_HOME, "Home" },
	{ SDLK_END, "End" },
	{ SDLK_RIGHT, "RightArrow"},
	{ SDLK_LEFT, "LeftArrow"},
	{ SDLK_DOWN, "DownArrow"},
	{ SDLK_UP, "Up Arrow"},
	{ SDLK_PAGEUP, "Page Up" },
	{ SDLK_PAGEDOWN, "Page Down" },
	{ SDLK_KP_DIVIDE, "KP /" },
	{ SDLK_KP_MULTIPLY, "KP *" },
	{ SDLK_KP_MINUS, "KP -" },
	{ SDLK_KP_PLUS, "KP +" },
	{ SDLK_KP_ENTER, "KP Enter" },
	{ SDLK_KP_PERIOD, "KP ." },
	{ SDLK_KP_0, "KP 0" },
	{ SDLK_KP_1, "KP 1" },
	{ SDLK_KP_2, "KP 2" },
	{ SDLK_KP_3, "KP 3" },
	{ SDLK_KP_4, "KP 4" },
	{ SDLK_KP_5, "KP 5" },
	{ SDLK_KP_6, "KP 6" },
	{ SDLK_KP_7, "KP 7" },
	{ SDLK_KP_8, "KP 8" },
	{ SDLK_KP_9, "KP 9" },
	{ SDLK_LCTRL, "LCtrl" },
	{ SDLK_LSHIFT, "LShift" },
	{ SDLK_LALT, "LAlt" },
	{ SDLK_LGUI, "LGui" },
	{ SDLK_RCTRL, "RCtrl" },
	{ SDLK_RSHIFT, "RShift" },
	{ SDLK_RALT, "RAlt" },
	{ SDLK_RGUI, "RGui" },
	{ SDLK_EXCLAIM, "!" },
	{ SDLK_QUOTE, "\'" },
	{ SDLK_COMMA, "," },
	{ SDLK_PERIOD, "." },
	{ SDLK_SLASH, "/" },
	{ SDLK_0, "0" },
	{ SDLK_1, "1" },
	{ SDLK_2, "2" },
	{ SDLK_3, "3" },
	{ SDLK_4, "4" },
	{ SDLK_5, "5" },
	{ SDLK_6, "6" },
	{ SDLK_7, "7" },
	{ SDLK_8, "8" },
	{ SDLK_9, "9" },
	{ SDLK_SEMICOLON, ";" },
	{ SDLK_EQUALS, "=" },
	{ SDLK_MINUS, "-" },
	{ SDLK_LEFTBRACKET, "[" },
	{ SDLK_RIGHTBRACKET, "]" },
	{ SDLK_BACKSLASH, "\\" },
	{ SDLK_BACKQUOTE, "`" },
	{ SDLK_a, "A" },
	{ SDLK_b, "B" },
	{ SDLK_c, "C" },
	{ SDLK_d, "D" },
	{ SDLK_e, "E" },
	{ SDLK_f, "F" },
	{ SDLK_g, "G" },
	{ SDLK_h, "H" },
	{ SDLK_i, "I" },
	{ SDLK_j, "J" },
	{ SDLK_k, "K" },
	{ SDLK_l, "L" },
	{ SDLK_m, "M" },
	{ SDLK_n, "N" },
	{ SDLK_o, "O" },
	{ SDLK_p, "P" },
	{ SDLK_q, "Q" },
	{ SDLK_r, "R" },
	{ SDLK_s, "S" },
	{ SDLK_t, "T" },
	{ SDLK_u, "U" },
	{ SDLK_v, "V" },
	{ SDLK_w, "W" },
	{ SDLK_x, "X" },
	{ SDLK_y, "Y" },
	{ SDLK_z, "Z" },
	{ SDLK_F1, "F1" },
	{ SDLK_F2, "F2" },
	{ SDLK_F3, "F3" },
	{ SDLK_F4, "F4" },
	{ SDLK_F5, "F5" },
	{ SDLK_F6, "F6" },
	{ SDLK_F7, "F7" },
	{ SDLK_F8, "F8" },
	{ SDLK_F9, "F9" },
	{ SDLK_F10, "F10" },
	{ SDLK_F11, "F11" },
	{ SDLK_F12, "F12" }
};

std::unordered_map<SDL_GameControllerButton, std::string> controller_keybinds =
{
	{ SDL_CONTROLLER_BUTTON_A, "A" },
	{ SDL_CONTROLLER_BUTTON_B, "B" },
	{ SDL_CONTROLLER_BUTTON_X, "X" },
	{ SDL_CONTROLLER_BUTTON_Y, "Y" },
	{ SDL_CONTROLLER_BUTTON_BACK, "Select" },
	{ SDL_CONTROLLER_BUTTON_GUIDE, "Guide" },
	{ SDL_CONTROLLER_BUTTON_START, "Start" },
	{ SDL_CONTROLLER_BUTTON_LEFTSTICK, "L3" },
	{ SDL_CONTROLLER_BUTTON_RIGHTSTICK, "R3" },
	{ SDL_CONTROLLER_BUTTON_LEFTSHOULDER, "L1" },
	{ SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, "R1" },
	{ SDL_CONTROLLER_BUTTON_DPAD_UP, "Up" },
	{ SDL_CONTROLLER_BUTTON_DPAD_DOWN, "Down" },
	{ SDL_CONTROLLER_BUTTON_DPAD_LEFT, "Left" },
	{ SDL_CONTROLLER_BUTTON_DPAD_RIGHT, "Right" },
	{ SDL_CONTROLLER_BUTTON_MISC1, "Misc" },
	{ SDL_CONTROLLER_BUTTON_PADDLE1, "Paddle 1" },  
	{ SDL_CONTROLLER_BUTTON_PADDLE2, "Paddle 2" },  
	{ SDL_CONTROLLER_BUTTON_PADDLE3, "Paddle 3" },  
	{ SDL_CONTROLLER_BUTTON_PADDLE4, "Paddle 4" },  
	{ SDL_CONTROLLER_BUTTON_TOUCHPAD, "Touchpad" }
};
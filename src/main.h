#pragma once

#include <ctime>
#include <filesystem>
#include <fstream>
#include <io.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <stack>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <thread>
#include <windows.h>
#include <shellapi.h>
#include <unordered_map>
#include <vector>
#include <commdlg.h>

#include "CRC.h"
#include "Emulator.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "json.hpp"
#include "Keybinds.h"
#include "SDL.h"
#include "SDL_syswm.h"
#include "SDL_image.h"
#include "pugixml.hpp"


struct game
{
	std::string CRC32 = "";
	std::string name = "";
	std::string alt_name = "";
	std::string publisher = "";
	std::string system = "";
	std::string time_played = "";
	std::string last_played_date = "";
	std::string last_played_time = "";
	bool has_preview = false;
};

struct config
{
	bool open_file = false;
	bool skip_metadata = false;
	bool start_games_fullscreen = false;
	bool fullscreen_on = false;
	bool hide_UI = false;
	bool fast_forward;
	bool toggle_fast_forward;
	int screenshot_number = 0;
	int volume = 100;
	int pulse1_volume = 100;
	int pulse2_volume = 100;
	int triangle_volume = 100;
	int noise_volume = 100;
	int DMC_volume = 100;
	controls keyboard;
	controls controller;
	hotkeys keyboard_hotkeys;
};
config settings;
config temp_settings;

// Settings
void saveSettings();
void loadSettings(const std::string& file_path);
void handleKeybindRemap(int type);
void setVolume(Emulator& emu);
void toggleFullscreen(SDL_Window*& window);

std::string computeCRC32(const std::string& file_path);
std::string loadROM(HWND hwnd);
SDL_Surface* loadImageAsSurface(SDL_Window*& window, const std::string& path);
std::vector<std::string> getNESFiles(const std::string& directory);

std::string getGameNameFromCRC(const pugi::xml_document& doc, const std::string& crc);
std::string getGameAltNameFromCRC(const pugi::xml_document& doc, const std::string& crc);
std::string getGamePublisherFromCRC(const pugi::xml_document& doc, const std::string& crc);
std::string getGameSystemFromCRC(const pugi::xml_document& doc, const std::string& crc);
std::string getCurrentDate();
std::string getCurrentTime();

void updatePlayTime(const std::string& folder_path, int duration);
void updateLastPlayedTime(const std::string& folder_path, game& game);

int ensureReadAndWritePerms(const std::string& folder_path);
bool ensureFolderPathExists(const std::string& folder_path);
void savePreview(const std::string& path, Emulator& emu);
void saveImage(const char* file_name, SDL_Renderer* renderer, uint8_t* frame_buffer);



void SetupImGuiStyle();

std::string current_directory = "";

bool done = false;
bool emulator_running = false;
bool emulator_paused = false;
bool action_disabled = true;
bool show_settings_window = false;

uint8_t selected_settings = 0;
std::string game_path;
std::string game_CRC32;
int current_game_index;

bool keybind_remap = false;
bool keybind_selected = false;
int selected_keybind = 0xFFFF;
int key_pressed = 0;

Emulator emulator;
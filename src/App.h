#pragma once
#include <filesystem>

#include "Emulator.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "json.hpp"
#include "Keybinds.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_syswm.h"
#include "utils.h"
#define anemoia_version "Version: 1.1.0"

class App
{
public:
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
		bool fast_forward = false;
		bool toggle_fast_forward = false;
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

	App();
	~App();
	int init(SDL_Window*& window, SDL_Renderer*& renderer);
	void shutdown();
	void determineWindow();
	void drawMenuBar(bool& done);
	void drawStatusBar();
	void drawGame();
	void drawSettings();
	void renderUI();
	void renderDebugAPU();
	void toggleFullscreen();
	void handleSDLEvents(bool& done);
	void saveSettings();
	void loadSettings(const std::string& file_path);
private:
	std::string loadROM(HWND hwnd);
	std::string saveState(HWND hwnd);
	std::string loadState(HWND hwnd);

	void handleKeybindRemap(int type);
	void setVolume(Emulator& emu);
	void SetupImGuiStyle();
	void saveImage(const char* file_name, SDL_Renderer* renderer, uint8_t* frame_buffer);

	void updatePlayTime(const std::string& folder_path, int duration);
	void updateLastPlayedTime(const std::string& folder_path, game& game);
	void savePreview(const std::string& path, Emulator& emu);

	// Fonts
	ImFontConfig font_cfg;
	ImVector<ImWchar> ranges;
	ImFontGlyphRangesBuilder builder;
	ImFont* font_size_24;
	ImFont* bold_font;
	ImFont* japanese_font;

	// Settings
	bool emulator_running = false;
	bool emulator_paused = false;
	uint8_t selected_settings = 0;
	int selected_keybind = 0xFFFF;
	int key_pressed = 0;
	bool keybind_selected = false;
	bool keybind_remap = false;
	bool show_settings_window = false;
	bool action_disabled = true;

	SDL_Texture* controller_image;
	std::string game_path;
	int current_game_index;

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	ImGuiIO* io = nullptr;
	SDL_Texture* missing_preview;
	ImGuiContext* main_context = NULL;
	std::unique_ptr<uint8_t[]> image_binary = std::make_unique<uint8_t[]>(256 * 240 * 3);
	std::vector<game> games;
	std::vector<SDL_Texture*> ROM_previews;
	std::vector<std::string> games_list;
	SDL_Joystick* joystick = NULL;

	std::string current_directory = "";
	std::string emulator_text = "";
	std::string game_CRC32 = "";
	bool show_text = false;
	float text_timer = 0.0f;
	config settings;
	config temp_settings;

	Emulator emulator;
};
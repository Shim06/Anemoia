#pragma once

#include <ctime>
#include <filesystem>
#include <fstream>
#include <io.h>
#include <iostream>
#include <memory>
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

#include "App.h"
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
#include "utils.h"
#include "pugixml.hpp"

std::string current_directory = "";
bool done = false;
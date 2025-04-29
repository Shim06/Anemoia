#pragma once
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <io.h>
#include <sstream>
#include <string>
#include <vector>
#include <stdio.h>

#include "pugixml.hpp"
#include "SDL.h"
#include "SDL_image.h"
#include "CRC.h"
#include "windows.h"

std::vector<std::string> getNESFiles(const std::string& directory);
SDL_Surface* loadImageAsSurface(SDL_Window*& window, const std::string& path);

std::string computeCRC32(const std::string& file_path);
std::string getGameNameFromCRC(const pugi::xml_document& doc, const std::string& crc);
std::string getGameAltNameFromCRC(const pugi::xml_document& doc, const std::string& crc);
std::string getGamePublisherFromCRC(const pugi::xml_document& doc, const std::string& crc);
std::string getGameSystemFromCRC(const pugi::xml_document& doc, const std::string& crc);
std::string getCurrentDate();
std::string getCurrentTime();

int ensureReadAndWritePerms(const std::string& folder_path);
bool ensureFolderPathExists(const std::string& folder_path);
std::filesystem::path getExecutableDirectory();
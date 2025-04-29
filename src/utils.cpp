#include "utils.h"

// Get all .nes files and put the file names into a vector
std::vector<std::string> getNESFiles(const std::string& directory)
{
    std::vector<std::string> file_names;
    std::string path(directory);
    std::string ext(".nes");
    for (auto& p : std::filesystem::recursive_directory_iterator(path))
    {
        if (p.path().extension() == ext)
            file_names.push_back(p.path().stem().string());
    }

    return file_names;
}

// Computes the CRC32 of a NES file. Skips the header bytes.
std::string computeCRC32(const std::string& file_path)
{
    std::ifstream ifs(file_path, std::ios::binary);
    if (ifs.is_open())
    {
        // Create a buffer to read the file in chunks
        std::vector<char> buffer(1024);
        std::uint32_t crc;

        // Get the CRC32
        ifs.ignore(16);
        ifs.read(buffer.data(), buffer.size());
        crc = CRC::Calculate(static_cast<void*>(buffer.data()), ifs.gcount(), CRC::CRC_32());
        while (ifs.read(buffer.data(), buffer.size()))
        {
            crc = CRC::Calculate(static_cast<void*>(buffer.data()), ifs.gcount(), CRC::CRC_32(), crc);
        }
        crc = CRC::Calculate(static_cast<void*>(buffer.data()), ifs.gcount(), CRC::CRC_32(), crc);

        // Return CRC32 as a string
        std::ostringstream oss;
        oss << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << crc;
        return oss.str();
    }

    return "";
}


SDL_Surface* loadImageAsSurface(SDL_Window*& window, const std::string& path)
{
    //The final optimized image
    SDL_Surface* optimized_surface = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == NULL)
    {
        printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
    }
    else
    {
        //Convert surface to screen format
        optimized_surface = SDL_ConvertSurface(loadedSurface, SDL_GetWindowSurface(window)->format, 0);
        if (optimized_surface == NULL)
        {
            printf("Unable to optimize image %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
        }

        //Get rid of old loaded surface
        SDL_FreeSurface(loadedSurface);
    }

    return optimized_surface;
}

std::string getGameNameFromCRC(const pugi::xml_document& doc, const std::string& crc)
{
    // Traverse the XML document to find the cartridge with the specified CRC
    pugi::xml_node game_node;
    for (pugi::xml_node game : doc.child("database").children("game"))
    {
        for (pugi::xml_node cartridge : game.children("cartridge"))
        {
            if (cartridge.attribute("crc").as_string() == crc)
            {
                game_node = game;
                break;
            }
        }
        if (game_node) {
            break;
        }
    }

    if (!game_node)
        return "";

    // Retrieve the game name
    std::string game_name = game_node.attribute("name").as_string();
    return game_name;
}

std::string getGameAltNameFromCRC(const pugi::xml_document& doc, const std::string& crc)
{
    // Traverse the XML document to find the cartridge with the specified CRC
    pugi::xml_node game_node;
    for (pugi::xml_node game : doc.child("database").children("game"))
    {
        for (pugi::xml_node cartridge : game.children("cartridge"))
        {
            if (cartridge.attribute("crc").as_string() == crc)
            {
                game_node = game;
                break;
            }
        }
        if (game_node) {
            break;
        }
    }

    if (!game_node)
        return "";

    // Retrieve the game altname
    std::string game_alt_name = game_node.attribute("altname").as_string();
    return game_alt_name;
}

std::string getGamePublisherFromCRC(const pugi::xml_document& doc, const std::string& crc)
{
    // Traverse the XML document to find the cartridge with the specified CRC
    pugi::xml_node game_node;
    for (pugi::xml_node game : doc.child("database").children("game"))
    {
        for (pugi::xml_node cartridge : game.children("cartridge"))
        {
            if (cartridge.attribute("crc").as_string() == crc)
            {
                game_node = game;
                break;
            }
        }
        if (game_node) {
            break;
        }
    }

    if (!game_node)
        return "";

    // Retrieve the game publisher
    std::string game_publisher = game_node.attribute("publisher").as_string();
    return game_publisher;
}

std::string getGameSystemFromCRC(const pugi::xml_document& doc, const std::string& crc)
{
    // Traverse the XML document to find the cartridge with the specified CRC
    pugi::xml_node cartridge_node;
    for (pugi::xml_node game : doc.child("database").children("game"))
    {
        for (pugi::xml_node cartridge : game.children("cartridge"))
        {
            if (cartridge.attribute("crc").as_string() == crc)
            {
                cartridge_node = cartridge;
                break;
            }
        }
        if (cartridge_node) {
            break;
        }
    }

    if (!cartridge_node)
        return "";

    // Retrieve the game name
    std::string system = cartridge_node.attribute("system").as_string();
    return system;
}

// Returns DD/MM/YYYY Format
std::string getCurrentDate()
{
    auto time = std::chrono::system_clock::now();
    std::time_t legacy_time = std::chrono::system_clock::to_time_t(time);
    std::tm local_tm;
    localtime_s(&local_tm, &legacy_time);
    char date_buffer[30];
    std::strftime(date_buffer, sizeof(date_buffer), "%d/%m/%Y", &local_tm);
    std::string str(date_buffer);
    return str;
}

// Returns 12-hour HH:MM:SS AM/PM
std::string getCurrentTime()
{
    auto time = std::chrono::system_clock::now();
    std::time_t legacy_time = std::chrono::system_clock::to_time_t(time);
    std::tm local_tm;
    localtime_s(&local_tm, &legacy_time);
    char time_buffer[30];
    std::strftime(time_buffer, sizeof(time_buffer), "%I:%M:%S %p", &local_tm);
    std::string str(time_buffer);
    return str;
}

// Checks for read and write permissions in folder path 
// Returns 0 if user has read/write permissions
// Returns 1 if user does not have read/write permissions
int ensureReadAndWritePerms(const std::string& folder_path)
{
    if (_access(folder_path.c_str(), 4) != 0) // Check read permission
    {
        std::cout << "No read permission for " << folder_path << std::endl;
        return 1;
    }

    if (_access(folder_path.c_str(), 2) != 0) // Check write permission
    {
        std::cout << "No write permission for " << folder_path << std::endl;
        return 1;
    }

    return 0;
}

bool ensureFolderPathExists(const std::string& folder_path)
{
    // Check if the folder exists
    if (!std::filesystem::exists(folder_path))
    {
        // Create folder
        if (std::filesystem::create_directories(folder_path))
            return true;

        // If can't create folder return false
        else
            return false;
    }
    return true;
}

std::filesystem::path getExecutableDirectory() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();
}
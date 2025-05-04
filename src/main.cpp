#include "main.h"
#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

int main(int, char**)
{
    std::filesystem::path current_directory_path = getExecutableDirectory();
    current_directory = current_directory_path.string() + "\\";
    if (ensureReadAndWritePerms(current_directory) != 0)
    {
        MessageBox(NULL, L"Program does not have read/write permissions!", L"ERROR", MB_ICONERROR | MB_OK);
        return 1;
    }

    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    char* lastSlash = strrchr(exePath, '\\');
    if (lastSlash) {
        *lastSlash = '\0';
        SetCurrentDirectoryA(exePath);
    }
    else return 1;

    // Ensure necessary folders exist
    ensureFolderPathExists(current_directory + "games");
    ensureFolderPathExists(current_directory + "saves");
    ensureFolderPathExists(current_directory + "screenshots");
    ensureFolderPathExists(current_directory + "system");
    ensureFolderPathExists(current_directory + "states");

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return 1;
    }

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

#define MINIMUM_WINDOW_WIDTH 800
#define MINIMUM_WINDOW_HEIGHT 500

    // Create window with SDL_Renderer graphics context
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
        | SDL_WINDOW_SHOWN | SDL_WINDOW_MAXIMIZED);
    SDL_Window* settings_window = NULL;
    SDL_Window* window = SDL_CreateWindow("Anemoia", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MINIMUM_WINDOW_WIDTH, MINIMUM_WINDOW_HEIGHT, window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);// || SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr)
    {
        SDL_Log("Error creating SDL_Renderer!");
        return -1;
    }
    SDL_SetWindowMinimumSize(window, MINIMUM_WINDOW_WIDTH, MINIMUM_WINDOW_HEIGHT);

    // Main loop
    App app;
    if (app.init(window, renderer) != 0) return 1;
    while (!done)
    {
        // Handle input and file opening
        app.handleSDLEvents(done);

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        app.determineWindow();

        app.drawMenuBar(done);
        app.drawStatusBar();
        app.drawGame();
        app.drawSettings();
        app.renderUI();

        // Debug Render
        // app.renderDebugAPU();
    }
    app.shutdown();
    return 0;
}
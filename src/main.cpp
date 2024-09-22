#include "main.h"

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

int main(int, char**)
{
    std::filesystem::path current_directory_path = std::filesystem::current_path();
    current_directory = current_directory_path.string() + "\\";
    if (ensureReadAndWritePerms(current_directory) != 0)
    {
        MessageBox(NULL, L"Program does not have read/write permissions!", L"ERROR", MB_ICONERROR | MB_OK);
        return 1;
    }

    // Get .nes files in games directory
    ensureFolderPathExists(current_directory + "games");
    std::vector<std::string> games_list = getNESFiles((current_directory + "games"));

    std::vector<game> games;
    games.resize(games_list.size());
    for (int i = 0, size = static_cast<int>(games_list.size()); i < size; i++)
    {
        // Get CRC32 of the .nes files and ROM info from xml database
        games[i].CRC32 = computeCRC32(current_directory + "games\\" + games_list[i] + ".nes");

        // Load the XML database
        std::string xml = current_directory + "system\\nescartdb.xml";
        pugi::xml_document database;
        pugi::xml_parse_result result = database.load_file(xml.c_str());

        if (!result)
            std::cerr << "XML parsing error: " << result.description() << std::endl;

        std::string name = getGameNameFromCRC(database, games[i].CRC32);
        games[i].name = (name != "") ? name : games_list[i]; // If ROM in database, use game name. Else, use file name.
        games[i].alt_name = getGameAltNameFromCRC(database, games[i].CRC32);
        games[i].publisher = getGamePublisherFromCRC(database, games[i].CRC32);
        games[i].system = getGameSystemFromCRC(database, games[i].CRC32);

        // Load metadata for each ROM (time played, last played)
        std::string folder_path = current_directory + "system\\games\\" + games[i].CRC32 + "\\";
        ensureFolderPathExists(folder_path);
        if (!std::filesystem::exists(folder_path + "metadata.json"))
        {
            // If the json doesn't exist, create a new one with default values
            nlohmann::json metadata;

            metadata["time_played"] = "00:00:00";
            metadata["last_played_date"] = "Never";
            metadata["last_played_time"] = "Never";

            std::ofstream metadata_file(folder_path + "metadata.json");
            metadata_file << std::setw(4) << metadata;
            metadata_file.close();
        }
        // Get values from json file
        std::ifstream metadata_file(folder_path + "metadata.json");
        nlohmann::json metadata;
        metadata_file >> metadata;
        metadata_file.close();

        games[i].time_played = metadata["time_played"];
        games[i].last_played_date = metadata["last_played_date"];
        games[i].last_played_time = metadata["last_played_time"];
    }

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
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); //| SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr)
    {
        SDL_Log("Error creating SDL_Renderer!");
        return 1;
    }
    SDL_SetWindowMinimumSize(window, MINIMUM_WINDOW_WIDTH, MINIMUM_WINDOW_HEIGHT);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGuiContext* main_context = ImGui::CreateContext();    
    ImGui::SetCurrentContext(main_context);
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    io.IniFilename = NULL;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    SetupImGuiStyle();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // Load Font
    ImFontConfig font_cfg;
    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    font_cfg.GlyphExtraSpacing.x = 0.0f;
    font_cfg.RasterizerDensity = 3.0f;
    io.Fonts->AddFontFromFileTTF((current_directory + "fonts\\roboto\\roboto-regular.ttf").c_str(), 16.0f * 1.5f, &font_cfg);
    ImFont* bold_font = io.Fonts->AddFontFromFileTTF((current_directory + "fonts\\roboto\\roboto-bold.ttf").c_str(), 18.0f * 1.5f, &font_cfg);

    for (wchar_t i = 0x2150; i <= 0x218F; i++) // UTF-8 Number Forms
        builder.AddChar(i);
    builder.AddRanges(io.Fonts->GetGlyphRangesJapanese());
    builder.BuildRanges(&ranges);
    ImFont* japanese_font = io.Fonts->AddFontFromFileTTF((current_directory + "fonts\\noto_sans_jp\\notosansjp-bold.ttf").c_str(),
                                                          20.0f * 1.5f, &font_cfg, ranges.Data);

    loadSettings(current_directory + "system\\settings.json");

    // Load image previews for ROMs
    std::unique_ptr<uint8_t[]> image_binary = std::make_unique<uint8_t[]>(256 * 240 * 3);
    std::vector<SDL_Texture*> ROM_previews;

    SDL_Texture* missing_preview = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 256, 240);
    std::ifstream ifs;
    ifs.open("system\\missing.fb", std::ifstream::binary); // Loads missing preview image
    if (ifs.is_open())
    {
        ifs.read(reinterpret_cast<char*>(image_binary.get()), 256 * 240 * 3);
        SDL_UpdateTexture(missing_preview, NULL, image_binary.get(), 256 * 3);
        ifs.close();
    }
    for (int i = 0, size = static_cast<int>(games_list.size()); i < size; i++)
    {
        std::ifstream ifs;
        std::string file_path = current_directory + "system\\games\\" + games[i].CRC32 + "\\preview.fb";
        ifs.open(file_path, std::ifstream::binary);
        if (ifs.is_open())
        {
            SDL_Texture* game_preview = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 256, 240);
            ROM_previews.push_back(game_preview);
            ifs.read(reinterpret_cast<char*>(image_binary.get()), 256 * 240 * 3);
            SDL_UpdateTexture(game_preview, NULL, image_binary.get(), 256 * 3);
            games[i].has_preview = true;
        }
        else
            ROM_previews.push_back(missing_preview);
    }

    // Load controller images
    SDL_Surface* controller_surface = loadImageAsSurface(window, current_directory + "system\\nes_controller.png");
    if (controller_surface == NULL)
    {
        printf("Unable to load image! SDL_image Error: %s\n", IMG_GetError());
        return 1;
    }
    SDL_Texture* controller_image = SDL_CreateTextureFromSurface(renderer, controller_surface); 
    if (controller_surface == NULL)
    {
        printf("Unable to load image! SDL_image Error: %s\n", IMG_GetError());
        return 1;
    }

    // Initialize controls 
    SDL_Joystick* joystick;
    SDL_JoystickEventState(SDL_ENABLE);
    joystick = SDL_JoystickOpen(0);

    // Main loop
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    while (!done)
    {
        // Handle input
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;

            switch (event.type)
            {
            case SDL_QUIT:
                done = true;
                break;

            case SDL_KEYDOWN:
                if (keybind_remap)
                {
                    key_pressed = event.key.keysym.sym;
                    keybind_remap = false;
                    keybind_selected = true;
                    break;
                }
                else if (event.key.keysym.sym == SDLK_o && (event.key.keysym.mod & KMOD_CTRL))
                {
                    settings.open_file = true;
                }

                else if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    SDL_SetWindowFullscreen(window, 0);
                    settings.fullscreen_on = false;
                    settings.hide_UI = false;
                }
                else if (event.key.keysym.sym == settings.keyboard_hotkeys.toggle_fullscreen)
                {
                    toggleFullscreen(window);
                }

                else if (show_settings_window || !emulator_running) break;


                else if (event.key.keysym.sym == settings.keyboard_hotkeys.screenshot)
                {
                    if (emulator.getFrame() != NULL)
                    {
                        ensureFolderPathExists(current_directory + "screenshots");
                        saveImage(("screenshots\\Screenshot (" + std::to_string(settings.screenshot_number) + ").png").c_str(),
                            renderer, emulator.ptr_frame_buffer);
                    }
                }

                else if (event.key.keysym.sym == settings.keyboard_hotkeys.pause)
                {
                    if (!emulator_paused) { emulator.pause(); emulator_paused = true; }
                    else { emulator.unpause(); emulator_paused = false; }
                }

                else if (event.key.keysym.sym == settings.keyboard_hotkeys.reset)
                    emulator.reset();

                else if (event.key.keysym.sym == settings.keyboard_hotkeys.show_UI)
                    settings.hide_UI = false;

                else if (event.key.keysym.sym == settings.keyboard_hotkeys.mute)
                    emulator.toggleMute();

                else if (event.key.keysym.sym == settings.keyboard_hotkeys.fast_forward)
                    settings.fast_forward = true;

                else if (event.key.keysym.sym == settings.keyboard_hotkeys.toggle_fast_forward)
                    settings.toggle_fast_forward = !settings.toggle_fast_forward;

                else if (event.key.keysym.sym == settings.keyboard.A)
                    emulator.setController1State(Emulator::controllerControls::A, true);

                else if (event.key.keysym.sym == settings.keyboard.B)
                    emulator.setController1State(Emulator::controllerControls::B, true);

                else if (event.key.keysym.sym == settings.keyboard.Up)
                    emulator.setController1State(Emulator::controllerControls::Up, true);

                else if (event.key.keysym.sym == settings.keyboard.Down)
                    emulator.setController1State(Emulator::controllerControls::Down, true);

                else if (event.key.keysym.sym == settings.keyboard.Left)
                    emulator.setController1State(Emulator::controllerControls::Left, true);

                else if (event.key.keysym.sym == settings.keyboard.Right)
                    emulator.setController1State(Emulator::controllerControls::Right, true);

                else if (event.key.keysym.sym == settings.keyboard.Select)
                    emulator.setController1State(Emulator::controllerControls::Select, true);

                else if (event.key.keysym.sym == settings.keyboard.Start)
                    emulator.setController1State(Emulator::controllerControls::Start, true);

                break;

            case SDL_KEYUP:
                if (event.key.keysym.sym == settings.keyboard_hotkeys.fast_forward)
                    settings.fast_forward = false;

                else if (event.key.keysym.sym == settings.keyboard.A)
                    emulator.setController1State(Emulator::controllerControls::A, false);

                else if (event.key.keysym.sym == settings.keyboard.B)
                    emulator.setController1State(Emulator::controllerControls::B, false);

                else if (event.key.keysym.sym == settings.keyboard.Up)
                    emulator.setController1State(Emulator::controllerControls::Up, false);

                else if (event.key.keysym.sym == settings.keyboard.Down)
                    emulator.setController1State(Emulator::controllerControls::Down, false);

                else if (event.key.keysym.sym == settings.keyboard.Left)
                    emulator.setController1State(Emulator::controllerControls::Left, false);

                else if (event.key.keysym.sym == settings.keyboard.Right)
                    emulator.setController1State(Emulator::controllerControls::Right, false);

                else if (event.key.keysym.sym == settings.keyboard.Select)
                    emulator.setController1State(Emulator::controllerControls::Select, false);

                else if (event.key.keysym.sym == settings.keyboard.Start)
                    emulator.setController1State(Emulator::controllerControls::Start, false);

                break;

            case SDL_JOYAXISMOTION:
                if (event.jaxis.value > 3200)
                {
                    if (event.jaxis.axis == 0)
                        emulator.setController1State(Emulator::controllerControls::Right, true);
                    if (event.jaxis.axis == 1)
                        emulator.setController1State(Emulator::controllerControls::Down, true);
                }
                else if (event.jaxis.value < -3200)
                {
                    if (event.jaxis.axis == 0)
                        emulator.setController1State(Emulator::controllerControls::Left, true);
                    if (event.jaxis.axis == 1)
                        emulator.setController1State(Emulator::controllerControls::Up, true);
                }
                else
                {
                    if (event.jaxis.axis == 0)
                    {
                        emulator.setController1State(Emulator::controllerControls::Right, false);
                        emulator.setController1State(Emulator::controllerControls::Left, false);

                    }
                    if (event.jaxis.axis == 1)
                    {
                        emulator.setController1State(Emulator::controllerControls::Up, false);
                        emulator.setController1State(Emulator::controllerControls::Down, false);
                    }
                }
                break;

            case SDL_JOYBUTTONDOWN:
                if (keybind_remap)
                {
                    key_pressed = event.jbutton.button;
                    keybind_remap = false;
                    keybind_selected = true;
                    break;
                }
                
                else if (event.jbutton.button == settings.controller.A)
                    emulator.setController1State(Emulator::controllerControls::A, true);

                else if (event.jbutton.button == settings.controller.B)
                    emulator.setController1State(Emulator::controllerControls::B, true);

                else if (event.jbutton.button == settings.controller.Up)
                    emulator.setController1State(Emulator::controllerControls::Up, true);

                else if (event.jbutton.button == settings.controller.Down)
                    emulator.setController1State(Emulator::controllerControls::Down, true);

                else if (event.jbutton.button == settings.controller.Left)
                    emulator.setController1State(Emulator::controllerControls::Left, true);

                else if (event.jbutton.button == settings.controller.Right)
                    emulator.setController1State(Emulator::controllerControls::Right, true);

                else if (event.jbutton.button == settings.controller.Select)
                    emulator.setController1State(Emulator::controllerControls::Select, true);

                else if (event.jbutton.button == settings.controller.Start)
                    emulator.setController1State(Emulator::controllerControls::Start, true);

                break;

            case SDL_JOYBUTTONUP:
                if (event.jbutton.button == settings.controller.A)
                    emulator.setController1State(Emulator::controllerControls::A, false);

                else if (event.jbutton.button == settings.controller.B)
                    emulator.setController1State(Emulator::controllerControls::B, false);

                else if (event.jbutton.button == settings.controller.Up)
                    emulator.setController1State(Emulator::controllerControls::Up, false);

                else if (event.jbutton.button == settings.controller.Down)
                    emulator.setController1State(Emulator::controllerControls::Down, false);

                else if (event.jbutton.button == settings.controller.Left)
                    emulator.setController1State(Emulator::controllerControls::Left, false);

                else if (event.jbutton.button == settings.controller.Right)
                    emulator.setController1State(Emulator::controllerControls::Right, false);

                else if (event.jbutton.button == settings.controller.Select)
                    emulator.setController1State(Emulator::controllerControls::Select, false);

                else if (event.jbutton.button == settings.controller.Start)
                    emulator.setController1State(Emulator::controllerControls::Start, false);

                break;
            }

        }
        if (settings.toggle_fast_forward || settings.fast_forward) emulator.fastForward();
        else emulator.normalSpeed();

        if (settings.open_file)
        {

            settings.open_file = false;
            SDL_SysWMinfo info;
            SDL_VERSION(&info.version);
            SDL_GetWindowWMInfo(window, &info);
            std::string rom_path = loadROM(info.info.win.window);
            if (rom_path != "")
            {
                if (emulator_running)
                {
                    if (!settings.skip_metadata)
                    {
                        std::string folder_path = current_directory + "system\\games\\" + games[current_game_index].CRC32 + "\\";
                        emulator.dumpSave(current_directory + "saves\\" + games[current_game_index].CRC32 + ".sav");
                        updatePlayTime(folder_path, emulator.getElapsedTime());
                        updateLastPlayedTime(folder_path, games[current_game_index]);
                    }
                    else
                        emulator.dumpSave(current_directory + "saves\\" + game_CRC32 + ".sav");
                }

                emulator_running = true;
                settings.skip_metadata = true;
                game_CRC32 = computeCRC32(rom_path);
                if (emulator.start(rom_path, renderer) == 1)
                {
                    emulator_running = false;
                    emulator.stop();
                    SDL_SetWindowTitle(window, "Anemoia");
                }
                else
                {
                    emulator.loadSave(current_directory + "saves\\" + game_CRC32 + ".sav");
                    setVolume(emulator);

                    // To Do: Seperate threads for emulation and GUI
                    //std::thread emulation_thread(&Emulator::emulate, &emulator);
                    //emulation_thread.detach();
                }
                // emulator.emulate(rom_path, current_directory);
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (!emulator_running || show_settings_window)
            ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y - ImGui::GetFrameHeight()));
        else
            ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, ImGui::GetFrameHeight()));

        ImGui::SetNextWindowPos({ 0, 0 });

        if (!settings.hide_UI)
        {
            ImGui::Begin("##NoLabel", nullptr, ImGuiWindowFlags_NoTitleBar
                | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus);
        }

        // Menu bar
        if (!settings.hide_UI && ImGui::BeginMenuBar())
        {
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2196f, 0.2196f, 0.2196f, 1.0f));
            // File menu
            if (ImGui::BeginMenu("File"))
            {

                // Load rom button
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                    settings.open_file = true;

                ImGui::Separator();

                // Open game folder button
                if (ImGui::MenuItem("Open Game Folder"))
                {
                    ShellExecuteA(NULL, "open", (current_directory + "games").c_str(), NULL, NULL, SW_SHOWDEFAULT);
                }
                ImGui::Separator();

                // Exit Button
                if (ImGui::MenuItem("Exit", "Alt+F4"))
                {
                    ImGui_ImplSDLRenderer2_Shutdown();
                    ImGui_ImplSDL2_Shutdown();
                    ImGui::DestroyContext(main_context);

                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                    SDL_Quit();
                    return 0;
                }
                ImGui::EndMenu();
            }

            // Options menu
            if (ImGui::BeginMenu("Options"))
            {

                if (ImGui::MenuItem("Toggle Fullscreen", keyboard_keybinds[static_cast<SDL_KeyCode>(settings.keyboard_hotkeys.toggle_fullscreen)].c_str()))
                    toggleFullscreen(window);

                if (ImGui::MenuItem("Start Games in Fullscreen Mode", nullptr, &settings.start_games_fullscreen))
                    saveSettings();

                ImGui::Separator();

                if (ImGui::MenuItem("Settings"))
                {
                    temp_settings = settings;
                    show_settings_window = true;
                }
                ImGui::EndMenu();
            }

            // Actions menu
            if (action_disabled)
                ImGui::BeginDisabled();
            if (ImGui::BeginMenu("Actions"))
            {

                if (!emulator_paused)
                {
                    if (ImGui::MenuItem("Pause", keyboard_keybinds[static_cast<SDL_KeyCode>(settings.keyboard_hotkeys.pause)].c_str()))
                    {
                        emulator.pause();
                        emulator_paused = true;
                    }
                }
                else
                {
                    if (ImGui::MenuItem("Resume", keyboard_keybinds[static_cast<SDL_KeyCode>(settings.keyboard_hotkeys.pause)].c_str()))
                    {
                        emulator.unpause();
                        emulator_paused = false;
                    }
                }

                if (ImGui::MenuItem("Reset", keyboard_keybinds[static_cast<SDL_KeyCode>(settings.keyboard_hotkeys.reset)].c_str()))
                {
                    emulator.reset();
                }

                if (ImGui::MenuItem("Stop Emulation"))
                {
                    emulator_running = false;
                    if (!settings.skip_metadata)
                        emulator.dumpSave(current_directory + "saves\\" + games[current_game_index].CRC32 + ".sav");
                    else
                        emulator.dumpSave(current_directory + "saves\\" + game_CRC32 + ".sav");
                    emulator.stop();

                    if (!settings.skip_metadata)
                    {
                        // Save game preview
                        std::string folder_path = current_directory + "system\\games\\" + games[current_game_index].CRC32 + "\\";
                        savePreview(folder_path, emulator);
                        updateLastPlayedTime(folder_path, games[current_game_index]);

                        // Reload game previews
                        if (games[current_game_index].has_preview)
                            SDL_DestroyTexture(ROM_previews[current_game_index]);
                        SDL_Texture* game_preview = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 256, 240);
                        ROM_previews[current_game_index] = game_preview;
                        size_t buffer_size = 256 * 240 * 3;
                        std::memcpy(image_binary.get(), emulator.ptr_frame_buffer, buffer_size);
                        SDL_UpdateTexture(game_preview, NULL, image_binary.get(), 256 * 3);


                        updatePlayTime(folder_path, emulator.getElapsedTime());
                        std::ifstream metadata_file(folder_path + "metadata.json");
                        if (metadata_file.is_open())
                        {
                            nlohmann::json metadata;
                            metadata_file >> metadata;
                            metadata_file.close();
                            games[current_game_index].time_played = metadata["time_played"];
                        }
                    }

                    SDL_SetWindowTitle(window, "Anemoia");
                }
                ImGui::Separator();

                if (ImGui::MenuItem("Toggle Mute", keyboard_keybinds[static_cast<SDL_KeyCode>(settings.keyboard_hotkeys.mute)].c_str()))
                {
                    emulator.toggleMute();
                }

                if (ImGui::MenuItem("Take Screenshot", keyboard_keybinds[static_cast<SDL_KeyCode>(settings.keyboard_hotkeys.screenshot)].c_str()))
                {
                    if (emulator.getFrame() != NULL)
                    {
                        ensureFolderPathExists(current_directory + "screenshots");
                        saveImage(("screenshots\\Screenshot (" + std::to_string(settings.screenshot_number) + ").png").c_str(),
                                    renderer, emulator.ptr_frame_buffer);
                    }
                }
               
                if (ImGui::MenuItem("Hide UI", keyboard_keybinds[static_cast<SDL_KeyCode>(settings.keyboard_hotkeys.show_UI)].c_str()))
                {
                    settings.hide_UI = true;
                }
                ImGui::EndMenu();
            }
            if (action_disabled)
            {
                ImGui::EndDisabled();
            }
            if (!emulator_running) action_disabled = true;

            // Help Menu
            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("Github"))
                {
                    ShellExecuteA(NULL, "open", "https://github.com/Shim06/Anemoia", NULL, NULL, SW_SHOWDEFAULT);
                }

                // if (ImGui::MenuItem("About")) {} // To do
                ImGui::EndMenu();
            }

            // End the menu bar
            ImGui::PopStyleColor();
            ImGui::EndMenuBar();
        }


        // Status bar
        if (!settings.hide_UI && !show_settings_window)
        {
            ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
            float height = ImGui::GetFrameHeight();

            ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.1765f, 0.1765f, 0.1765f, 1.0f));
            if (ImGui::BeginViewportSideBar("##MainStatusBar", viewport, ImGuiDir_Down, height, window_flags)) {
                if (ImGui::BeginMenuBar()) {

                    // IMGUI FPS -> ImGui::Text("%.2f FPS (%.2f ms)", io.Framerate, 1000.0f / io.Framerate);
                    if (emulator_running)
                    {
                        double emulator_fps = emulator.getAvgFPS();
                        ImGui::Text("%.2f FPS (%.2f ms)", emulator_fps, 1000.0f / emulator_fps);
                    }

                    const char* text = "Version: 1.0.0";
                    ImVec2 text_size = ImGui::CalcTextSize(text);

                    // Position the text on the right side
                    ImVec2 text_pos = ImVec2(ImGui::GetWindowWidth() - text_size.x - 10.0f, ImGui::GetCursorPosY());
                    ImGui::SetCursorPos(text_pos);
                    ImGui::Text(text);
                    ImGui::EndMenuBar();
                }
            }
            ImGui::End();
            ImGui::PopStyleColor();
        }
    

        // Games list
        if (!emulator_running)
        {
            float table_width = ImGui::GetContentRegionAvail().x;
            ImGuiTableFlags table_flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoPadInnerX | ImGuiTableFlags_NoPadOuterX;
            if (ImGui::BeginTable("Games", 1, table_flags))
            {
                // Set up columns with fixed widths
                ImGui::TableSetupColumn("Column 1", ImGuiTableColumnFlags_WidthFixed, table_width);

                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 4.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
                for (int i = 0, size = static_cast<int>(games_list.size()); i < size; i++)
                {
                    const char* game_title = games[i].name.c_str();
                    const char* game_alt_name = games[i].alt_name.c_str();
                    const char* game_publisher = games[i].publisher.c_str();
                    const char* game_CRC32 = games[i].CRC32.c_str();
                    const char* game_system = games[i].system.c_str();
                    const char* time_played = games[i].time_played.c_str();
                    const char* last_played_date = games[i].last_played_date.c_str();
                    const char* last_played_time = games[i].last_played_time.c_str();

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Button(("##" + games[i].CRC32).c_str(), ImVec2(-FLT_MIN, 151));

                    // If a game is selected
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                    {
                        emulator_running = true;
                        action_disabled = false;
                        settings.skip_metadata = false;
                        game_path = (current_directory + "games\\" + games_list[i] + ".nes");
                        current_game_index = i;
                        if (settings.start_games_fullscreen)
                        {
                            settings.hide_UI = true;
                            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                        }

                        std::string window_title = "Anemoia | " + std::string(games[i].alt_name != "" ? game_alt_name : game_title);
                        SDL_SetWindowTitle(window, window_title.c_str());

                        std::string folder_path = current_directory + "system\\games\\" + games[i].CRC32 + "\\";
                        updateLastPlayedTime(folder_path, games[i]);
                        
                    }

                    ImVec2 button_size = ImGui::GetItemRectSize();
                    ImVec2 button_pos = ImGui::GetItemRectMin();
                    ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    // Game image
                    if (missing_preview == NULL)
                    {
                        printf("Failed to create texture! SDL_Error: %s\n", SDL_GetError());
                    }
                    else
                    {
                        draw_list->AddImage((void*)ROM_previews[i],
                            ImVec2(button_pos.x + (button_size.x * 0.008f), button_pos.y + (button_size.y * 0.1f)),
                            ImVec2(button_pos.x + (button_size.x * 0.008f) + 121.5f, button_pos.y + (button_size.y * 0.9f)),
                            ImVec2(0,0)), ImVec2(1,1);

                    }
                    
                    static constexpr float padding = 135.75f;
                    // Game Title & Alt Title
                    if (games[i].alt_name != "")
                    {
                        ImGui::PushFont(japanese_font);
                        draw_list->AddText(ImVec2(button_pos.x + (button_size.x * 0.008f) + padding,
                            button_pos.y + (button_size.y * 0.135f)),
                            color, game_alt_name);
                        ImGui::PopFont();

                        ImGui::PushFont(bold_font);
                        draw_list->AddText(ImVec2(button_pos.x + (button_size.x * 0.008f) + padding,
                            button_pos.y + (button_size.y * 0.34f)),
                            color, game_title);
                        ImGui::PopFont();
                    }
                    else
                    {
                        ImGui::PushFont(bold_font);
                        draw_list->AddText(ImVec2(button_pos.x + (button_size.x * 0.008f) + padding,
                            button_pos.y + (button_size.y * 0.135f)), // 0.34f
                            color, game_title);
                        ImGui::PopFont();
                    }

                    // Game Publisher
                    draw_list->AddText(ImVec2(button_pos.x + (button_size.x * 0.008f) + padding,
                                       button_pos.y + (button_size.y * (games[i].alt_name != "" ? 0.53f : 0.34f))), // 0.53f
                                       color, game_publisher);

                    static constexpr float padding2 = 265.05f * 1.2f;
                    draw_list->AddLine(ImVec2(button_pos.x + (button_size.x * 1.0f) - padding2, button_pos.y + (button_size.y * 0.1f)),
                                       ImVec2(button_pos.x + (button_size.x * 1.0f) - padding2, button_pos.y + (button_size.y * 0.9f)),
                                       IM_COL32(0x4D, 0x4D, 0x4D, 255), 1.2f);


                    // Game CRC32 
                    draw_list->AddText(ImVec2(button_pos.x + (button_size.x * 1.0f) - padding2 + (13.08f * 1.5f),
                                       button_pos.y + (button_size.y * 0.135f)),
                                       color, game_CRC32);
                    // Game system
                    draw_list->AddText(ImVec2(button_pos.x + (button_size.x * 1.0f) - padding2 + (13.08f * 1.5f),
                                       button_pos.y + (button_size.y * 0.34f)),
                                       color, game_system);

                    // Time Played
                    ImVec2 text_size = ImGui::CalcTextSize(time_played);
                    draw_list->AddText(ImVec2((button_pos.x + button_size.x) - text_size.x - 15.0f,
                                       button_pos.y + (button_size.y * 0.135f)),
                                       color, time_played);
                    // Last Played Date
                    text_size = ImGui::CalcTextSize(last_played_date);
                    draw_list->AddText(ImVec2((button_pos.x + button_size.x) - text_size.x - 15.0f,
                                       button_pos.y + (button_size.y * 0.34f)),
                                       color, last_played_date);
                    // Last Played Time
                    text_size = ImGui::CalcTextSize(last_played_time);
                    draw_list->AddText(ImVec2((button_pos.x + button_size.x) - text_size.x - 15.0f,
                                       button_pos.y + (button_size.y * 0.545f)),
                                       color, last_played_time);

                }
                ImGui::EndTable();
                ImGui::PopStyleVar(3);
            }
        }

        else
        {
            // Emulation window
            ImGuiWindowFlags emulator_window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse
                                                   | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove 
                                                   | ImGuiWindowFlags_NoBringToFrontOnFocus;
            float height = settings.hide_UI ? io.DisplaySize.y : io.DisplaySize.y - (ImGui::GetFrameHeight() * 2);
            ImVec2 pos = settings.hide_UI ? ImVec2(0, 0) : ImVec2(0, ImGui::GetFrameHeight());
            ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, height));
            ImGui::SetNextWindowPos(pos);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 1.0f));
            ImGui::Begin("Anemoia", nullptr, emulator_window_flags);

            if (!emulator.isRunning())
            {
                if (emulator.start(game_path, renderer) == 1)
                {
                    emulator_running = false;
                    emulator.stop();
                    SDL_SetWindowTitle(window, "Anemoia");
                }
                else
                {
                    emulator.loadSave(current_directory + "saves\\" + games[current_game_index].CRC32 + ".sav");
                    setVolume(emulator);

                    // To Do: Seperate threads for emulation and GUI
                    //std::thread emulation_thread(&Emulator::emulate, &emulator);
                    //emulation_thread.detach();
                }
            }
            else
                emulator.emulate();

            if (emulator.getFrame() != nullptr)
            {
                // Scale NES frame to window
                ImVec2 window_size = ImGui::GetContentRegionAvail();
                float scaled_width, scaled_height;
                float aspect_ratio = 256.0f / 240.0f;

                if (window_size.x / window_size.y > aspect_ratio)
                {
                    scaled_height = window_size.y;
                    scaled_width = scaled_height * aspect_ratio;
                }
                else
                {
                    scaled_width = window_size.x;
                    scaled_height = scaled_width / aspect_ratio;
                }

                // Center image
                float pos_x = (window_size.x - scaled_width) * 0.5f;
                float pos_y = (window_size.y - scaled_height) * 0.5f;
                ImGui::SetCursorPos(ImVec2(pos_x, pos_y));


                ImGui::Image((void*)emulator.getFrame(), ImVec2(scaled_width, scaled_height));
            }

            ImGui::End();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }

        if (!settings.hide_UI)
            ImGui::End();

        // Settings Window
        if (show_settings_window)
        {
            ImGui::SetNextWindowSize(io.DisplaySize);
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12549f, 0.12549f, 0.12549f, 1.0f));
            if (ImGui::Begin("Settings", &show_settings_window, ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
            {

                ImGui::SetCursorPosX(io.DisplaySize.x * 0.0035f);
                float table_width = io.DisplaySize.x * 0.1522f;
                ImGuiTableFlags table_flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoPadInnerX | ImGuiTableFlags_NoPadOuterX;
                if (ImGui::BeginTable("settingstable", 1, table_flags))
                {
                    // Set up columns with fixed widths
                    ImGui::TableSetupColumn("Column 1", ImGuiTableColumnFlags_WidthFixed, table_width);

                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 4.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12549f, 0.12549f, 0.12549f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.17647f, 0.17647f, 0.17647f, 1.0f));
                    std::string settings_buttons[] = {"Hotkeys", "Input", "Audio"};
                    for (uint8_t i = 0, size = static_cast<uint8_t>(std::size(settings_buttons)); i < size; i++)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        static constexpr float button_height = 52.5f;
                        if (selected_settings == i)
                        {
                            ImDrawList* draw_list = ImGui::GetWindowDrawList();
                            ImVec2 pos = ImGui::GetCursorPos();
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.17647f, 0.17647f, 0.17647f, 1.0f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15647f, 0.15647f, 0.15647f, 1.0f));

                            if (ImGui::Button(settings_buttons[i].c_str(), ImVec2(-FLT_MIN, 52.5f)))
                            {
                                keybind_remap = false;
                                selected_keybind = 0xFFFF;
                                selected_settings = i;
                            }

                            ImGui::PopStyleColor(2);
                            draw_list->AddRectFilled(ImVec2(pos.x, pos.y + (button_height * 0.27f)), ImVec2(pos.x + 4.95f, pos.y + (button_height * 0.73f)),
                                                     IM_COL32(25, 233, 255, 255), 4.0f);
                        }
                        else if (ImGui::Button(settings_buttons[i].c_str(), ImVec2(-FLT_MIN, button_height)))
                        {
                            keybind_remap = false;
                            selected_keybind = 0xFFFF;
                            selected_settings = i;
                        }
                        
                    }

                    ImGui::EndTable();
                    ImGui::PopStyleVar(4);
                    ImGui::PopStyleColor(2);
                }
                
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1569f, 0.1569f, 0.1569f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.251f, 0.251f, 0.251f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.5f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.1595f, io.DisplaySize.y * 0.0169f));
                if (ImGui::BeginChild("content_window", ImVec2(io.DisplaySize.x * 0.833f, io.DisplaySize.y * 0.905f), true))
                {
                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    ImVec2 pos;
                    ImVec2 offset;
                    ImVec2 size = ImGui::GetWindowSize();
                    ImVec2 hotkey_button_size = ImVec2(120, 45);
                    ImVec2 keybind_button_size = ImVec2(size.x / 13.325f, size.y / 19.8f);

                    // Hotkeys
                    static const std::string text[] =
                    { "Fast Forward:", "Toggle Fast Forward:", "Reset:", "Screenshot:", "Enter Fullscreen:", "Show UI:", "Pause:", "Mute:" };
                    std::string key[] =
                    { 
                        keyboard_keybinds[static_cast<SDL_KeyCode>(temp_settings.keyboard_hotkeys.fast_forward)],
                        keyboard_keybinds[static_cast<SDL_KeyCode>(temp_settings.keyboard_hotkeys.toggle_fast_forward)],
                        keyboard_keybinds[static_cast<SDL_KeyCode>(temp_settings.keyboard_hotkeys.reset)],
                        keyboard_keybinds[static_cast<SDL_KeyCode>(temp_settings.keyboard_hotkeys.screenshot)],
                        keyboard_keybinds[static_cast<SDL_KeyCode>(temp_settings.keyboard_hotkeys.toggle_fullscreen)],
                        keyboard_keybinds[static_cast<SDL_KeyCode>(temp_settings.keyboard_hotkeys.show_UI)],
                        keyboard_keybinds[static_cast<SDL_KeyCode>(temp_settings.keyboard_hotkeys.pause)],
                        keyboard_keybinds[static_cast<SDL_KeyCode>(temp_settings.keyboard_hotkeys.mute)]
                    };
                    static constexpr int hotkey[] =
                    { KEY_FAST_FORWARD, KEY_TOGGLE_FAST_FORWARD, KEY_RESET, KEY_SCREENSHOT, KEY_TOGGLE_FULLSCREEN, KEY_SHOW_UI, KEY_PAUSE, KEY_MUTE };


                    // Keybinds
                    static constexpr int keybind[] =
                    { KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_SELECT, KEY_START, KEY_B, KEY_A };
                    ImVec2 keybind_position[] =
                    { 
                        {size.x * 0.196f, size.y * 0.44f},
                        {size.x * 0.196f, size.y * 0.615f},
                        {size.x * 0.15f, size.y * 0.526f},
                        {size.x * 0.242f, size.y * 0.526f},
                        {size.x * 0.378f, size.y * 0.595f},
                        {size.x * 0.487f, size.y * 0.595f},
                        {size.x * 0.625f, size.y * 0.595f},
                        {size.x * 0.728f, size.y * 0.595f},
                    };
                    std::string keyboard_keybind_key[] =
                    {
                        keyboard_keybinds[static_cast<SDL_KeyCode>(temp_settings.keyboard.Up)],
                        keyboard_keybinds[static_cast<SDL_KeyCode>(temp_settings.keyboard.Down)],
                        keyboard_keybinds[static_cast<SDL_KeyCode>(temp_settings.keyboard.Left)],
                        keyboard_keybinds[static_cast<SDL_KeyCode>(temp_settings.keyboard.Right)],
                        keyboard_keybinds[static_cast<SDL_KeyCode>(temp_settings.keyboard.Select)],
                        keyboard_keybinds[static_cast<SDL_KeyCode>(temp_settings.keyboard.Start)],
                        keyboard_keybinds[static_cast<SDL_KeyCode>(temp_settings.keyboard.B)],
                        keyboard_keybinds[static_cast<SDL_KeyCode>(temp_settings.keyboard.A)],
                    };
                    std::string controller_keybind_key[] =
                    {
                        controller_keybinds[static_cast<SDL_GameControllerButton>(temp_settings.controller.Up)],
                        controller_keybinds[static_cast<SDL_GameControllerButton>(temp_settings.controller.Down)],
                        controller_keybinds[static_cast<SDL_GameControllerButton>(temp_settings.controller.Left)],
                        controller_keybinds[static_cast<SDL_GameControllerButton>(temp_settings.controller.Right)],
                        controller_keybinds[static_cast<SDL_GameControllerButton>(temp_settings.controller.Select)],
                        controller_keybinds[static_cast<SDL_GameControllerButton>(temp_settings.controller.Start)],
                        controller_keybinds[static_cast<SDL_GameControllerButton>(temp_settings.controller.B)],
                        controller_keybinds[static_cast<SDL_GameControllerButton>(temp_settings.controller.A)],
                    };

                    // Audio
                    static const std::string audio_text[] =
                    { "Master Volume:", "Pulse 1 Channel:", "Pulse 2 Channel:",
                      "Triangle Channel:", "Noise Channel: ", "DMC Channel:" };
                    int* volume[] =
                    { &temp_settings.volume, &temp_settings.pulse1_volume, &temp_settings.pulse2_volume,
                      &temp_settings.triangle_volume, &temp_settings.noise_volume, &temp_settings.DMC_volume };

                    static const char* items[] = { "Keyboard", "Controller" };
                    static int keybind_type = 0;


                    switch (selected_settings)
                    {
                    case 0: // Keyboard Hotkey Settings
                        handleKeybindRemap(0);

                        ImGui::PushFont(japanese_font);
                        ImGui::Text("Keyboard Hotkeys");
                        ImGui::PopFont();

                        offset = ImVec2(300.0f, -9.0f);
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15.0f);

                        for (uint8_t i = 0, size = static_cast<uint8_t>(std::size(text)); i < size; i++)
                        {
                            pos = ImGui::GetCursorPos();
                            ImGui::Text(text[i].c_str());
                            ImGui::SetCursorPos(ImVec2(pos.x + offset.x, pos.y + offset.y));
                            if (keybind_remap && selected_keybind == hotkey[i])
                            {
                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.098f, 0.8745f, 1.0f, 1.0f));
                                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.098f, 0.79608f, 0.9098f, 1.0f));
                                if (ImGui::Button(key[i].c_str(), hotkey_button_size))
                                {
                                    keybind_remap = true;
                                    selected_keybind = hotkey[i];
                                }
                                ImGui::PopStyleColor(2);
                            }
                            else if (ImGui::Button(key[i].c_str(), hotkey_button_size))
                            {
                                keybind_remap = true;
                                selected_keybind = hotkey[i];
                            }
                        }

                        break;

                    case 1: // Input Settings
                        handleKeybindRemap(keybind_type);

                        pos = ImGui::GetCursorPos();
                        ImGui::PushFont(japanese_font);
                        ImGui::Text("Input");
                        offset = ImGui::CalcTextSize("Input");
                        ImGui::PopFont();

                        ImGui::SetCursorPos(ImVec2(pos.x + 80, pos.y - 5));
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(15, 10));
                        ImGui::SetNextItemWidth((size.x * 0.95f) - 80);
                        ImGui::Combo("##Keybind", &keybind_type, items, static_cast<int>(std::size(items)));
                        ImGui::PopStyleVar();


                        pos = { io.DisplaySize.x * 0.1595f, io.DisplaySize.y * 0.0169f };

                        // Image size 1280x527
                        draw_list->AddImage((ImTextureID)controller_image,
                                             ImVec2((size.x / 2 + pos.x) - (size.x / 2.5f), (size.y / 2 + pos.y) - (size.y / 3.529296875f)),
                                             ImVec2((size.x / 2 + pos.x) + (size.x / 2.5f), (size.y / 2 + pos.y) + (size.y / 3.529296875f)));

                        for (uint8_t i = 0, size = static_cast<uint8_t>(std::size(keybind)); i < size; i++)
                        {
                            const char* text = "";
                            if (keybind_type == 0) text = keyboard_keybind_key[i].c_str();
                            else if (keybind_type == 1) text = controller_keybind_key[i].c_str();

                            ImGui::SetCursorPos(keybind_position[i]);
                            if (keybind_remap && selected_keybind == keybind[i])
                            {
                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.098f, 0.8745f, 1.0f, 1.0f));
                                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.098f, 0.79608f, 0.9098f, 1.0f));

                                if (ImGui::Button(text, keybind_button_size))
                                {
                                    keybind_remap = true;
                                    selected_keybind = keybind[i];
                                }
                                ImGui::PopStyleColor(2);
                            }
                            else if (ImGui::Button(text, keybind_button_size))
                            {
                                keybind_remap = true;
                                selected_keybind = keybind[i];
                            }
                        }
                        break;
                        

                    case 2: // Audio Settings
                        ImGui::PushFont(japanese_font);
                        ImGui::Text("Audio");
                        ImGui::PopFont();

                        offset = ImVec2(210.0f, 0.0f);
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15.0f);

                        for (uint8_t i = 0, array_size = static_cast<uint8_t>(std::size(audio_text)); i < array_size; i++)
                        {
                            pos = ImGui::GetCursorPos();
                            ImGui::Text(audio_text[i].c_str());
                            ImGui::SetCursorPos(ImVec2(pos.x + offset.x, pos.y + offset.y));

                            std::string str = std::to_string(*volume[i]);
                            const char* text = str.c_str();
                            ImVec2 text_size = ImGui::CalcTextSize(text);
                            ImVec2 text_pos = ImVec2(ImGui::GetCursorPosX() + size.x * 0.4f - text_size.x, ImGui::GetCursorPosY() + 30.0f);

                            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.11206691f, 0.1262157f, 0.154506445f, 1.0f));
                            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.11206691f, 0.1262157f, 0.154506445f, 1.0f));
                            ImGui::PushItemWidth(size.x * 0.4f);
                            ImGui::SliderInt(("##" + audio_text[i]).c_str(), volume[i], 0, 100);
                            ImGui::PopItemWidth();
                            ImGui::PopStyleColor(2);

                            ImGui::SetCursorPos(text_pos);
                            ImGui::Text(text);
                        }
                        break;

                    default: break;
                    }
                }
                ImGui::EndChild();

                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(3);

              

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.17647f, 0.17647f, 0.17647f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.196078f, 0.196078f, 0.196078f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, io.DisplaySize.y * 0.0121704f);
                ImVec2 button_size = ImVec2(io.DisplaySize.x * 0.0625f * 1.2f, io.DisplaySize.y * 0.04564f * 1.2f);
                //if (selected_keybind == 0) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.196078f, 0.196078f, 0.196078f, 1.0f));
                ImGui::SetCursorPos(ImVec2(io.DisplaySize.x - button_size.x - 15.0f, io.DisplaySize.y * 0.93f));
                if (ImGui::Button("Apply", button_size))
                {
                    settings = temp_settings;
                    saveSettings();
                    setVolume(emulator);
                }

                ImGui::SetCursorPos(ImVec2(io.DisplaySize.x - (button_size.x * 2) - (15.0f * 2), io.DisplaySize.y * 0.93f));
                if (ImGui::Button("Cancel", button_size)) 
                {
                    show_settings_window = false;
                }

                ImGui::SetCursorPos(ImVec2(io.DisplaySize.x - (button_size.x * 3) - (15.0f * 3), io.DisplaySize.y * 0.93f));
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.098f, 0.8745f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.098f, 0.79608f, 0.9098f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                if (ImGui::Button("Confirm", button_size))
                {
                    settings = temp_settings;
                    saveSettings();
                    setVolume(emulator);
                    show_settings_window = false;
                }
                ImGui::PopStyleColor(3);
                ImGui::PopStyleVar(2);
                
                ImGui::PopStyleColor(2);
            }
            ImGui::End();
            ImGui::PopStyleColor();
        }
        
        // Debug APU
        /*
        ImGui::Begin("APU", nullptr);
        ImGui::Text("Output Level: %i", +emulator.nes.apu.DMC.output_unit.output_level);
        ImGui::Text("Remaining Bits: %i", +emulator.nes.apu.DMC.output_unit.remaining_bits);
        ImGui::Text("Remaining Bytes: %i", +emulator.nes.apu.DMC.memory_reader.remaining_bytes);
        ImGui::Text("Shift Register: %i", +emulator.nes.apu.DMC.output_unit.shift_register);
        ImGui::Text("Silence Flag: %s", +emulator.nes.apu.DMC.output_unit.silence_flag ? "True" : "False");
        ImGui::Text("Loop Flag: %i", +emulator.nes.apu.DMC.loop_flag);
        ImGui::Text("Sample buffer: %i", +emulator.nes.apu.DMC.sample_buffer);
        ImGui::Text("Sample DMC: %i", +emulator.nes.apu.DMC_sample_byte);
        ImGui::Text("Sample address: %i", +emulator.nes.apu.DMC.sample_address);
        ImGui::Text("Memory Reader address: %i", +emulator.nes.apu.DMC.memory_reader.address);
        ImGui::Text("Sample length: %i", +emulator.nes.apu.DMC.sample_length);
        ImGui::Text("Timer: %i", +emulator.nes.apu.DMC.timer);
        ImGui::Text("DMA Alignment: %s", emulator.nes.apu.DMC_DMA_alignment ? "True" : "False");
        ImGui::Text("DMA Dummy: %s", emulator.nes.apu.DMC_DMA_dummy ? "True" : "False");
        ImGui::Text("DMA Reload: %s", emulator.nes.apu.DMC_DMA_reload ? "True" : "False");
        ImGui::End();
        */

        // Rendering
        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);

    }
    std::string folder_path = current_directory + "system\\games\\" + games[current_game_index].CRC32 + "\\";
    if (emulator_running)
    {
        if (!settings.skip_metadata)
        {
            emulator.dumpSave(current_directory + "saves\\" + games[current_game_index].CRC32 + ".sav");
            emulator.stop();
            updatePlayTime(folder_path, emulator.getElapsedTime());
            updateLastPlayedTime(folder_path, games[current_game_index]);
        }
        else
        {
            emulator.stop();
            emulator.dumpSave(current_directory + "saves\\" + game_CRC32 + ".sav");
        }
    }


    if (!settings.skip_metadata) 
        savePreview(folder_path, emulator);

    saveSettings();

    // Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext(main_context);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void saveSettings()
{
    std::ifstream settings_json_file(current_directory + "system\\settings.json");
    nlohmann::json settings_json;
    settings_json_file >> settings_json;

    settings_json["volume"] = settings.volume;
    settings_json["pulse1_volume"] = settings.pulse1_volume;
    settings_json["pulse2_volume"] = settings.pulse2_volume;
    settings_json["triangle_volume"] = settings.triangle_volume;
    settings_json["noise_volume"] = settings.noise_volume;
    settings_json["DMC_volume"] = settings.DMC_volume;
    settings_json["start_games_fullscreen"] = settings.start_games_fullscreen;
    settings_json["screenshot_number"] = settings.screenshot_number;

    settings_json["keyboard_A"] = settings.keyboard.A;
    settings_json["keyboard_B"] = settings.keyboard.B;
    settings_json["keyboard_Start"] = settings.keyboard.Start;
    settings_json["keyboard_Select"] = settings.keyboard.Select;
    settings_json["keyboard_Up"] = settings.keyboard.Up;
    settings_json["keyboard_Down"] = settings.keyboard.Down;
    settings_json["keyboard_Left"] = settings.keyboard.Left;
    settings_json["keyboard_Right"] = settings.keyboard.Right;

    settings_json["controller_A"] = settings.controller.A;
    settings_json["controller_B"] = settings.controller.B;
    settings_json["controller_Start"] = settings.controller.Start;
    settings_json["controller_Select"] = settings.controller.Select;
    settings_json["controller_Up"] = settings.controller.Up;
    settings_json["controller_Down"] = settings.controller.Down;
    settings_json["controller_Left"] = settings.controller.Left;
    settings_json["controller_Right"] = settings.controller.Right;

    settings_json["fast_forward"] = settings.keyboard_hotkeys.fast_forward;
    settings_json["toggle_fast_forward"] = settings.keyboard_hotkeys.toggle_fast_forward;
    settings_json["screenshot"] = settings.keyboard_hotkeys.screenshot;
    settings_json["show_UI"] = settings.keyboard_hotkeys.show_UI;
    settings_json["pause"] = settings.keyboard_hotkeys.pause;
    settings_json["mute"] = settings.keyboard_hotkeys.mute;
    settings_json["toggle_fullscreen"] = settings.keyboard_hotkeys.toggle_fullscreen;
    settings_json["reset"] = settings.keyboard_hotkeys.reset;

    std::ofstream file_stream(current_directory + "system\\settings.json");
    file_stream << std::setw(4) << settings_json << std::endl;
}

void loadSettings(const std::string& file_path)
{
    std::ifstream settings_file(file_path);
    if (!settings_file.is_open())
    {
        // If the json doesn't exist, create a new one with default values
        nlohmann::json settings;

        settings["volume"] = 100;
        settings["pulse1_volume"] = 100;
        settings["pulse2_volume"] = 100;
        settings["triangle_volume"] = 100;
        settings["noise_volume"] = 100;
        settings["DMC_volume"] = 100;
        settings["start_games_fullscreen"] = false;
        settings["screenshot_number"] = 0;

        settings["keyboard_A"] = SDLK_k;
        settings["keyboard_B"] = SDLK_j;
        settings["keyboard_Start"] = SDLK_m;
        settings["keyboard_Select"] = SDLK_n;
        settings["keyboard_Up"] = SDLK_w;
        settings["keyboard_Down"] = SDLK_s;
        settings["keyboard_Left"] = SDLK_a;
        settings["keyboard_Right"] = SDLK_d;

        settings["controller_A"] = SDL_CONTROLLER_BUTTON_B;
        settings["controller_B"] = SDL_CONTROLLER_BUTTON_A;
        settings["controller_Start"] = SDL_CONTROLLER_BUTTON_START;
        settings["controller_Select"] = SDL_CONTROLLER_BUTTON_BACK;
        settings["controller_Up"] = SDL_CONTROLLER_BUTTON_DPAD_UP;
        settings["controller_Down"] = SDL_CONTROLLER_BUTTON_DPAD_DOWN;
        settings["controller_Left"] = SDL_CONTROLLER_BUTTON_DPAD_LEFT;
        settings["controller_Right"] = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;

        settings["fast_forward"] = SDLK_LSHIFT;
        settings["toggle_fast_forward"] = SDLK_F1;
        settings["screenshot"] = SDLK_F8;
        settings["show_UI"] = SDLK_F4;
        settings["pause"] = SDLK_F5;
        settings["mute"] = SDLK_F2;
        settings["toggle_fullscreen"] = SDLK_F11;
        settings["reset"] = SDLK_t;

        std::ofstream newsettingsFile("system\\settings.json");
        newsettingsFile << std::setw(4) << settings;
        newsettingsFile.close();
    }

    // Get values from json file
    std::ifstream settings_json_file(file_path);
    nlohmann::json settings_json;
    settings_json_file >> settings_json;
    settings_json_file.close();

    settings.volume = settings_json["volume"];
    settings.pulse1_volume = settings_json["pulse1_volume"];
    settings.pulse2_volume = settings_json["pulse2_volume"];
    settings.triangle_volume = settings_json["triangle_volume"];
    settings.noise_volume = settings_json["noise_volume"];
    settings.DMC_volume = settings_json["DMC_volume"];
    settings.start_games_fullscreen = settings_json["start_games_fullscreen"];
    settings.screenshot_number = settings_json["screenshot_number"];

    settings.keyboard.A = settings_json["keyboard_A"];
    settings.keyboard.B = settings_json["keyboard_B"];
    settings.keyboard.Start = settings_json["keyboard_Start"];
    settings.keyboard.Select = settings_json["keyboard_Select"];
    settings.keyboard.Up = settings_json["keyboard_Up"];
    settings.keyboard.Down = settings_json["keyboard_Down"];
    settings.keyboard.Left = settings_json["keyboard_Left"];
    settings.keyboard.Right = settings_json["keyboard_Right"];

    settings.controller.A = settings_json["controller_A"];
    settings.controller.B = settings_json["controller_B"];
    settings.controller.Start = settings_json["controller_Start"];
    settings.controller.Select = settings_json["controller_Select"];
    settings.controller.Up = settings_json["controller_Up"];
    settings.controller.Down = settings_json["controller_Down"];
    settings.controller.Left = settings_json["controller_Left"];
    settings.controller.Right = settings_json["controller_Right"];

    settings.keyboard_hotkeys.fast_forward = settings_json["fast_forward"];
    settings.keyboard_hotkeys.toggle_fast_forward = settings_json["toggle_fast_forward"];
    settings.keyboard_hotkeys.screenshot = settings_json["screenshot"];
    settings.keyboard_hotkeys.show_UI = settings_json["show_UI"];
    settings.keyboard_hotkeys.pause = settings_json["pause"];
    settings.keyboard_hotkeys.mute = settings_json["mute"];
    settings.keyboard_hotkeys.toggle_fullscreen = settings_json["toggle_fullscreen"];
    settings.keyboard_hotkeys.reset = settings_json["reset"];

    temp_settings = settings;
}

// Handles keybind remaps.
// Pass 0 for keyboard keybinds
// Pass 1 for controller keybinds
void handleKeybindRemap(int type)
{
    if (!keybind_selected)
        return;

    if (type == 0)
    {
        // Return if key is not supported
        if (keyboard_keybinds.find(static_cast<SDL_KeyCode>(key_pressed)) == keyboard_keybinds.end())
        {
            keybind_selected = false; 
            selected_keybind = 0xFFFF;
            return;
        }

        keybind_selected = false;
        switch (selected_keybind)
        {
        case KEY_A:
            temp_settings.keyboard.A = key_pressed;
            break;

        case KEY_B:
            temp_settings.keyboard.B = key_pressed;
            break;

        case KEY_SELECT:
            temp_settings.keyboard.Select = key_pressed;
            break;

        case KEY_START:
            temp_settings.keyboard.Start = key_pressed;
            break;

        case KEY_UP:
            temp_settings.keyboard.Up = key_pressed;
            break;

        case KEY_DOWN:
            temp_settings.keyboard.Down = key_pressed;
            break;

        case KEY_LEFT:
            temp_settings.keyboard.Left = key_pressed;
            break;

        case KEY_RIGHT:
            temp_settings.keyboard.Right = key_pressed;
            break;

        case KEY_FAST_FORWARD:
            temp_settings.keyboard_hotkeys.fast_forward = key_pressed;
            break;

        case KEY_TOGGLE_FAST_FORWARD:
            temp_settings.keyboard_hotkeys.toggle_fast_forward = key_pressed;
            break;

        case KEY_SCREENSHOT:
            temp_settings.keyboard_hotkeys.screenshot = key_pressed;
            break;

        case KEY_SHOW_UI:
            temp_settings.keyboard_hotkeys.show_UI = key_pressed;
            break;

        case KEY_PAUSE:
            temp_settings.keyboard_hotkeys.pause = key_pressed;
            break;

        case KEY_MUTE:
            temp_settings.keyboard_hotkeys.mute = key_pressed;
            break;

        case KEY_TOGGLE_FULLSCREEN:
            temp_settings.keyboard_hotkeys.toggle_fullscreen = key_pressed;
            break;

        case KEY_RESET:
            temp_settings.keyboard_hotkeys.reset = key_pressed;
            break;
        }
        selected_keybind = 0xFFFF;
    }
    else if (type == 1)
    {
        // Return if key is not supported
        if (controller_keybinds.find(static_cast<SDL_GameControllerButton>(key_pressed)) == controller_keybinds.end())
        {
            keybind_selected = false;
            selected_keybind = 0xFFFF;
            return;
        }

        keybind_selected = false;
        switch (selected_keybind)
        {
        case KEY_A:
            temp_settings.controller.A = key_pressed;
            break;

        case KEY_B:
            temp_settings.controller.B = key_pressed;
            break;

        case KEY_SELECT:
            temp_settings.controller.Select = key_pressed;
            break;

        case KEY_START:
            temp_settings.controller.Start = key_pressed;
            break;

        case KEY_UP:
            temp_settings.controller.Up = key_pressed;
            break;

        case KEY_DOWN:
            temp_settings.controller.Down = key_pressed;
            break;

        case KEY_LEFT:
            temp_settings.controller.Left = key_pressed;
            break;

        case KEY_RIGHT:
            temp_settings.controller.Right = key_pressed;
            break;
        }
        selected_keybind = 0xFFFF;
    }
}

void setVolume(Emulator& emu)
{
    emu.setVolume(settings.volume);
    emu.setPulse1Volume(settings.pulse1_volume);
    emu.setPulse2Volume(settings.pulse2_volume);
    emu.setTriangleVolume(settings.triangle_volume);
    emu.setNoiseVolume(settings.noise_volume);
    emu.setDmcVolume(settings.DMC_volume);
}

void toggleFullscreen(SDL_Window*& window)
{
    settings.fullscreen_on = !settings.fullscreen_on;
    if (settings.fullscreen_on)
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    else
        SDL_SetWindowFullscreen(window, 0);

    if (emulator_running)
        settings.hide_UI = true;
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

std::string loadROM(HWND hwnd)
{
    OPENFILENAMEA ofn;
    CHAR szFile[260] = { 0 };
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "NES Files (*.nes)\0*.nes\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetOpenFileNameA(&ofn) == TRUE)
    {
        return ofn.lpstrFile;
    }

    return std::string();
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

void updatePlayTime(const std::string& folder_path, int duration)
{
    std::string time_played;
    ensureFolderPathExists(folder_path);

    nlohmann::json metadata;
    std::ifstream file(folder_path + "metadata.json");
    if (file.is_open())
    {
        // Get values from json file
        file >> metadata;
        file.close();

        time_played = metadata["time_played"];
        std::string delimiter = ":";
        int hours = std::stoi(time_played.substr(0, time_played.find(delimiter)));
        time_played.erase(0, time_played.find(delimiter) + delimiter.length());
        int minutes = std::stoi(time_played.substr(0, time_played.find(delimiter)));
        time_played.erase(0, time_played.find(delimiter) + delimiter.length());
        int seconds = std::stoi(time_played.substr(0, time_played.find(delimiter)));

        long total_time = (hours * 3600) + (minutes * 60) + seconds + duration;

        hours = total_time / 3600;
        minutes = (total_time % 3600) / 60;
        seconds = total_time % 60;

        std::stringstream time_stream;
        if (hours < 100)
            time_stream << std::setfill('0') << std::setw(2) << hours;
        else 
            time_stream << hours;
        time_stream << ":" << std::setfill('0') << std::setw(2) << minutes
                    << ":" << std::setfill('0') << std::setw(2) << seconds;

        time_played = time_stream.str();
    }

    std::ofstream file_output(folder_path + "metadata.json");
    if (file_output.is_open())
    {
        metadata["time_played"] = time_played;
        file_output << std::setw(4) << metadata;
        file_output.close();
    }
}

void updateLastPlayedTime(const std::string& folder_path, game& game)
{
    game.last_played_date = getCurrentDate();
    game.last_played_time = getCurrentTime();
    nlohmann::json metadata;
    std::ifstream file(folder_path + "metadata.json");
    if (!file.is_open())
        return;
    file >> metadata;
    file.close();

    std::ofstream output_file(folder_path + "metadata.json");
    if (!output_file.is_open())
        return;

    metadata["last_played_date"] = game.last_played_date;
    metadata["last_played_time"] = game.last_played_time;

    std::ofstream metadata_file(folder_path + "metadata.json");
    metadata_file << std::setw(4) << metadata;
    metadata_file.close();
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

void savePreview(const std::string& path, Emulator& emu)
{
    std::ofstream file(path + "preview.fb", std::ios::binary);

    uint16_t width = 256;
    uint8_t height = 240;
    size_t data_size = width * height * 3; // 3 bytes per pixel (RGB)
    file.write(reinterpret_cast<const char*>(emulator.ptr_frame_buffer), data_size);
    file.close();
}

void saveImage(const char* file_path, SDL_Renderer* renderer, uint8_t* frame_buffer) 
{
    settings.screenshot_number++;
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom
    (
        const_cast<void*>(static_cast<const void*>(frame_buffer)), 
        256,  
        240,        
        24,  
        (256 * 3),      
        0x000000FF, 
        0x0000FF00,
        0x00FF0000,
        0x00000000
    );

    if (!surface)
    {
        std::cout << "Failed to create surface: " << SDL_GetError() << std::endl;
    }
    else
    {
        IMG_SavePNG(surface, file_path);
    }
}

void SetupImGuiStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.5f;
    style.WindowPadding = ImVec2(12.0f, 12.0f);
    style.WindowBorderSize = 0.0f;
    style.WindowMinSize = ImVec2(20.0f, 20.0f);
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Right;
    style.ChildRounding = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupRounding = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameRounding = 11.89999961853027f;
    style.FrameBorderSize = 0.0f;
    style.FramePadding = ImVec2(15, 10);
    style.ItemSpacing = ImVec2(4.300000190734863f, 5.5f);
    style.ItemInnerSpacing = ImVec2(7.099999904632568f, 1.799999952316284f);
    style.CellPadding = ImVec2(12.10000038146973f, 9.199999809265137f);
    style.IndentSpacing = 0.0f;
    style.ColumnsMinSpacing = 4.900000095367432f;
    style.ScrollbarSize = 10.0f;
    style.ScrollbarRounding = 10.0f;
    style.TabRounding = 0.0f;
    style.TabBorderSize = 0.0f;
    style.TabMinWidthForCloseButton = 0.0f;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
    style.FrameRounding = 8.0f;
    style.GrabRounding = 20.0f;
    style.GrabMinSize = 30.0f;
    style.ItemSpacing = ImVec2(10, 10);
    style.PopupRounding = 8.0f;
    //style.ItemSpacing = ImVec2(10, 5);
    style.ScaleAllSizes(1.5f);

    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.2745098173618317f, 0.3176470696926117f, 0.4509803950786591f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.176f, 0.176f, 0.176f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.09250493347644806f, 0.100297249853611f, 0.1158798336982727f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.00f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1120669096708298f, 0.1262156516313553f, 0.1545064449310303f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.176f, 0.176f, 0.176f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.6235f, 0.6235f, 0.6235f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.6235f, 0.6235f, 0.6235f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.6235f, 0.6235f, 0.6235f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.9725490212440491f, 1.0f, 0.4980392158031464f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.098f, 0.8745f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.098f, 0.79608f, 0.9098f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.2235f, 0.2235f, 0.2235f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.2745f, 0.2745f, 0.2745f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1545050293207169f, 0.1545048952102661f, 0.1545064449310303f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.176f, 0.176f, 0.176f, 1.0f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.3137f, 0.3137f, 0.3137f, 1.0f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1459212601184845f, 0.1459220051765442f, 0.1459227204322815f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.9725490212440491f, 1.0f, 0.4980392158031464f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.999999463558197f, 1.0f, 0.9999899864196777f, 1.0f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1249424293637276f, 0.2735691666603088f, 0.5708154439926147f, 1.0f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.5215686559677124f, 0.6000000238418579f, 0.7019608020782471f, 1.0f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.03921568766236305f, 0.9803921580314636f, 0.9803921580314636f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8841201663017273f, 0.7941429018974304f, 0.5615870356559753f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.9570815563201904f, 0.9570719599723816f, 0.9570761322975159f, 1.0f);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
    style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
    style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.9356134533882141f, 0.9356129765510559f, 0.9356223344802856f, 1.0f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.266094446182251f, 0.2890366911888123f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.125f, 0.125f, 0.125f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.2196f, 0.2196f, 0.2196f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);

}
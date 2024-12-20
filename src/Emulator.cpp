#include "Emulator.h"

void Emulator::emulate()
{
    if (paused) return;

    if (elapsed_frames >= (60 * 3))
    {
        elapsed_frames = 0;
        fps_timer.start();
    }

    // Handle Game input
    nes.controller[0] = controller;

    // Main Emulation
    // Clocks the NES until a frame is complete
    do
    {
        nes.clock();

        // Fills the audio device with samples synthesized by the NES
        if (nes.apu.buffer_filled)
        {
            // Fill audio device with silence if there is no queued data
            // to prevent audio popping from closing and opening audio channel
            if (SDL_GetQueuedAudioSize(dev) < 512)
            {
                static constexpr int data[512] = { 0 };
                SDL_QueueAudio(dev, data, 512);
            }

            // Wait to prevent audio queue being overfilled with samples
            // Also locks emulation loop to ~60fps because the NES synthesizes 44100 audio samples/~60fps
            while (SDL_GetQueuedAudioSize(dev) > (512 * 2))
            {
                SDL_Delay(1);
            }

            if (audio_buffer_count >= audio_buffer_skip)
            {
                SDL_QueueAudio(dev, nes.apu.audio_buffer.get(), 512);
                audio_buffer_count = 0;
            }

            audio_buffer_count++;
            nes.apu.buffer_filled = false;
        }
    } while (!nes.ppu.frame_complete);
    nes.ppu.frame_complete = false;
    elapsed_frames++;

    renderFramebuffer(renderer, frame, ptr_frame_buffer);

    avg_FPS = elapsed_frames / (fps_timer.getTicks() / 1000.f);
    if (avg_FPS > 2000000) avg_FPS = 0;
}

int Emulator::start(const std::string& file_name, SDL_Renderer*& main_renderer)
{
    // Initialize Emulator
    renderer = main_renderer;
    start_time = std::chrono::steady_clock::now();
    if (!init_audio()) return 1;
 
    // Load game ROM
    cart = std::make_shared<Cartridge>(file_name);
    if (!cart->isValidMapper())
    {
        MessageBox(NULL, L"Game Mapper not supported!", L"ERROR", MB_ICONERROR | MB_OK);
        return 1;
    }
    nes.insertCartridge(cart);
    nes.reset();

    ptr_frame_buffer = nes.ppu.frame_buffer.get();

    SDL_PauseAudioDevice(dev, 0);
    fps_timer.start();
    running = true;
    return 0;
}

void Emulator::stop()
{
    running = false;
    end_time = std::chrono::steady_clock::now();
    fps_timer.stop();
    avg_FPS = 0;
    elapsed_frames = 0;
    SDL_ClearQueuedAudio(dev);
    SDL_PauseAudioDevice(dev, 1);
}

bool Emulator::init_audio()
{
    // Initialize want
    want.freq = 44100;
    want.format = AUDIO_U8;
    want.channels = 1;
    want.samples = 512;

    // Initialize SDL audio device
    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (dev == 0)
    {
        SDL_Log("Could not create SDL Audio device: %s", SDL_GetError());
        return false;
    }

    if ((want.format != have.format) ||
        (want.channels) != have.channels)
    {
        SDL_Log("Could not get desired audio spec");
        return false;
    }

    return true;
}

void Emulator::renderFramebuffer(SDL_Renderer*& renderer, SDL_Texture*& frame, uint8_t *framebuffer)
{
    if (frame == nullptr)
    {
        frame = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
        if (frame == NULL)
        {
            printf("Failed to create texture! SDL_Error: %s\n", SDL_GetError());
            return;
        }
    }
    SDL_UpdateTexture(frame, NULL, framebuffer, 256 * 3);
}


void Emulator::fastForward()
{
    audio_buffer_skip = 2;
}

void Emulator::normalSpeed()
{
    audio_buffer_skip = 1;
}

void Emulator::toggleMute()
{
    nes.apu.toggleMute();
}

SDL_Texture* Emulator::getFrame()
{
    std::lock_guard<std::mutex> lock(mtx);
    return frame;
}

void Emulator::setVolume(uint8_t volume)
{
    nes.apu.setVolume(volume);
}

void Emulator::setPulse1Volume(uint8_t volume)
{
    nes.apu.setPulse1Volume(volume);
}

void Emulator::setPulse2Volume(uint8_t volume)
{
    nes.apu.setPulse2Volume(volume);
}

void Emulator::setTriangleVolume(uint8_t volume)
{
    nes.apu.setTriangleVolume(volume);
}

void Emulator::setNoiseVolume(uint8_t volume)
{
    nes.apu.setNoiseVolume(volume);
}

void Emulator::setDmcVolume(uint8_t volume)
{
    nes.apu.setDmcVolume(volume);
}

bool Emulator::isRunning()
{
    return running;
}

void Emulator::setController1State(controllerControls bit, bool state)
{
    if (state == true)
        controller |= bit;
    else
        controller &= ~bit;
}

void Emulator::loadSave(const std::string& path)
{
    nes.cart->getMapper()->loadRAM(path);
}

void Emulator::dumpSave(const std::string& path)
{
    nes.cart->getMapper()->dumpRAM(path);
}

bool Emulator::loadState(const std::string& path, const std::string& CRC32)
{
    // Check if state file starts with "ANEMOIA"
    std::ifstream load_state(path, std::ios::binary);
    if (!load_state) return false;

    const char* header_text = "ANEMOIA";
    char text_buffer[7];
    load_state.read(text_buffer, sizeof(text_buffer));
    for (uint8_t i = 0; i < 7; i++)
    {
        if (text_buffer[i] != header_text[i])
            return false;
    }

    // Check if state file corresponds to the correct game
    // by checking the game CRC32 hash
    const char* hash = CRC32.c_str();
    char hash_buffer[8];
    load_state.read(hash_buffer, sizeof(hash_buffer));
    for (uint8_t i = 0; i < 7; i++)
    {
        if (hash_buffer[i] != hash[i])
            return false;
    }

    nes.resetClock();
    nes.apu.resetChannels();

    size_t cpu_dump_size;
    size_t ppu_dump_size;
    size_t cart_dump_size;
    size_t mapper_dump_size;
    load_state.read(reinterpret_cast<char*>(&cpu_dump_size), sizeof(cpu_dump_size));
    load_state.read(reinterpret_cast<char*>(&ppu_dump_size), sizeof(ppu_dump_size));
    load_state.read(reinterpret_cast<char*>(&cart_dump_size), sizeof(cart_dump_size));
    load_state.read(reinterpret_cast<char*>(&mapper_dump_size), sizeof(mapper_dump_size));

    std::vector<uint16_t> cpu_dump(cpu_dump_size);
    std::vector<uint16_t> ppu_dump(ppu_dump_size);
    std::vector<uint8_t> cart_dump(cart_dump_size);
    std::vector<uint8_t> mapper_dump(mapper_dump_size); 

    load_state.read(reinterpret_cast<char*>(&nes.cpu_ram), sizeof(nes.cpu_ram));
    load_state.read(reinterpret_cast<char*>(cpu_dump.data()), cpu_dump_size * sizeof(uint16_t));
    load_state.read(reinterpret_cast<char*>(ppu_dump.data()), ppu_dump_size * sizeof(uint16_t));
    load_state.read(reinterpret_cast<char*>(cart_dump.data()), cart_dump_size * sizeof(uint8_t));
    load_state.read(reinterpret_cast<char*>(mapper_dump.data()), mapper_dump_size * sizeof(uint8_t));

    load_state.close();

    nes.cpu.loadState(cpu_dump);
    nes.ppu.loadState(ppu_dump);
    nes.cart->loadState(cart_dump);
    nes.cart->getMapper()->loadState(mapper_dump);
    return true;
}

bool Emulator::saveState(const std::string& path, const std::string& CRC32)
{
    std::cout << "YO1";
    std::ofstream save_state(path, std::ios::binary | std::ios::trunc);
    if (!save_state) return false;

    // Write "ANEMOIA" and game CRC32 hash to file header 
    const char* text = "ANEMOIA";
    const char* hash = CRC32.c_str();
    save_state.write(text, strlen(text));
    save_state.write(hash, strlen(hash));

    std::vector<uint16_t> cpu_dump = nes.cpu.dumpState();
    std::vector<uint16_t> ppu_dump = nes.ppu.dumpState();
    std::vector<uint8_t> cart_dump = nes.cart->dumpState();
    std::vector<uint8_t> mapper_dump = nes.cart->getMapper()->dumpState();

    // Write size of each dump
    size_t cpu_dump_size = cpu_dump.size();
    size_t ppu_dump_size = ppu_dump.size();
    size_t cart_dump_size = cart_dump.size();
    size_t mapper_dump_size = mapper_dump.size();

    std::cout << "YO";

    save_state.write(reinterpret_cast<char*>(&cpu_dump_size), sizeof(cpu_dump_size));
    save_state.write(reinterpret_cast<char*>(&ppu_dump_size), sizeof(ppu_dump_size));
    save_state.write(reinterpret_cast<char*>(&cart_dump_size), sizeof(cart_dump_size));
    save_state.write(reinterpret_cast<char*>(&mapper_dump_size), sizeof(mapper_dump_size));

    // Dump state
    save_state.write(reinterpret_cast<char*>(&nes.cpu_ram), sizeof(nes.cpu_ram));
    for (auto const& x : cpu_dump)
        save_state.write(reinterpret_cast<const char*>(&x), sizeof(x));
    for (auto const& x : ppu_dump)
        save_state.write(reinterpret_cast<const char*>(&x), sizeof(x));
    for (auto const& x : cart_dump) save_state << x;
    for (auto const& x : mapper_dump) save_state << x;
    /*
    save_state.write(reinterpret_cast<char*>(cpu_dump.data()), cpu_dump.size() * sizeof(uint16_t));
    save_state.write(reinterpret_cast<char*>(ppu_dump.data()), ppu_dump.size() * sizeof(uint16_t));
    save_state.write(reinterpret_cast<char*>(cart_dump.data()), cart_dump.size() * sizeof(uint8_t));
    save_state.write(reinterpret_cast<char*>(mapper_dump.data()), mapper_dump.size() * sizeof(uint8_t));
    */
    save_state.close();
    return true;
}

int Emulator::getElapsedTime()
{
    std::chrono::duration<double, std::milli> elapsed_time = end_time - start_time;
    return static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(elapsed_time).count());
}

double Emulator::getAvgFPS()
{
    return avg_FPS;
}

void Emulator::reset()
{
    nes.reset();
    fps_timer.stop();
    fps_timer.start();
    avg_FPS = 0;
    elapsed_frames = 0;
    SDL_ClearQueuedAudio(dev);
}

void Emulator::pause()
{
    paused = true;
    SDL_PauseAudioDevice(dev, 1);
    fps_timer.pause();
}

void Emulator::unpause()
{
    paused = false;
    SDL_PauseAudioDevice(dev, 0);
    fps_timer.unpause();
}
#include "host_interface.h"

HostInterface::HostInterface() : 
    core([this](float fps) {
        UpdateTitle(fps);
    }) {
    
}

bool HostInterface::Initialise() {
    if (SDL_Init(SDL_INIT_VIDEO) > 0) {
        log_warn("error initialising SDL!");
        return false;
    }

    u32 window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;

    window = SDL_CreateWindow("otterstation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, window_flags);

    if (window == NULL) {
        log_warn("error initialising SDL_Window!");
        return false;
    }

    // create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {
        log_warn("error initialising SDL_Renderer");
        return false;
    }

    // create the top and bottom textures
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 640, 480);

    if (texture == NULL) {
        log_warn("error initialising SDL_Texture");
        return false;
    }

    // if the function has reached this far it has successfully initialised
    return true;
}

void HostInterface::Run(std::string path) {
    core.Reset();
    core.SetState(CoreState::Running);

    while (true) {
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                Shutdown();
                return;
            }
        }
    }
}

void HostInterface::Shutdown() {
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(texture);
    SDL_Quit();
}

void HostInterface::UpdateTitle(float fps) {
    // char window_title[40];
    // snprintf(window_title, 40, "yuugen [%0.2f FPS | %0.2f ms]", fps, 1000.0 / fps);
    // SDL_SetWindowTitle(window, window_title);
}
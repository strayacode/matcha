#pragma once

#include <core/core.h>
#include <string>
#include <SDL2/SDL.h>
#include <common/log.h>
#include <common/types.h>

class HostInterface {
public:
    HostInterface();

    bool Initialise();
    void Run(std::string path);
    void Shutdown();
    void UpdateTitle(float fps);

private:
    Core core;

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    SDL_Event event;

    int window_size = 1;
};
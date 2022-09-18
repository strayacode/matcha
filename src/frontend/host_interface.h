#pragma once

#include <common/types.h>
#include <common/log.h>
#include <core/core.h>
#include <string.h>
#include <stdlib.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imfilebrowser.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include "frontend/debugger/ee.h"
#include "frontend/debugger/iop.h"

class HostInterface {
public:
    HostInterface();

    bool Initialise();
    void Run();
    void Shutdown();
    void SetupStyle();
    void UpdateTitle(float fps);
    void TogglePause();

    Core core;
private:
    void HandleInput();
    void RenderMenubar();
    
    const char* glsl_version = "#version 330";

    SDL_Window* window;
    SDL_GLContext gl_context;
    bool show_demo_window = true;
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
    bool running = true;
    ImGui::FileBrowser file_dialog;
    EEDebugger ee_debugger;
    IOPDebugger iop_debugger;
};
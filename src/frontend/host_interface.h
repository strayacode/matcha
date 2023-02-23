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

    // static constexpr ImVec4 colour_blue = ImVec4(0.173f, 0.306f, 0.945f, 1.000f);
    // static constexpr ImVec4 colour_blue = ImVec4(0.100f, 0.400f, 0.750f, 1.000f);
    static constexpr ImVec4 colour_blue = ImVec4(0.000f, 0.471f, 0.843f, 1.000f);
    static constexpr ImVec4 colour_darker_blue = ImVec4(0.100f, 0.400f, 0.750f, 1.000f);
    static constexpr ImVec4 colour_clearer_blue = ImVec4(0.000f, 0.471f, 0.843f, 0.780f);
    static constexpr ImVec4 colour_black = ImVec4(0.075f, 0.075f, 0.075f, 1.000f);
    static constexpr ImVec4 colour_white = ImVec4(0.857f, 0.857f, 0.857f, 1.000f);
    static constexpr ImVec4 colour_green = ImVec4(0.405f, 0.776f, 0.421f, 1.000f);
    static constexpr ImVec4 colour_yellow = ImVec4(1.000f, 0.933f, 0.345f, 1.000f);
    static constexpr ImVec4 colour_lighter_lighter_lighter_lighter_grey = ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
    static constexpr ImVec4 colour_lighter_lighter_lighter_grey = ImVec4(0.260f, 0.260f, 0.260f, 1.000f);
    static constexpr ImVec4 colour_lighter_lighter_grey = ImVec4(0.220f, 0.220f, 0.220f, 1.000f);
    static constexpr ImVec4 colour_lighter_grey = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
    static constexpr ImVec4 colour_grey = ImVec4(0.140f, 0.140f, 0.140f, 1.000f);
    static constexpr ImVec4 colour_darker_grey = ImVec4(0.093f, 0.093f, 0.093f, 1.000f);
    
private:
    void HandleInput();
    void RenderMenubar();
    void boot(const std::string& path);
    
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
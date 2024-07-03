#pragma once

#include "common/types.h"
#include "common/log.h"
#include "common/games_list.h"
#include "core/core.h"
#include <string.h>
#include <stdlib.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imfilebrowser.h"
#include <SDL.h>
#include <SDL_opengl.h>

class HostInterface {
public:
    HostInterface();

    bool initialise();
    void run();
    void shutdown();
    void TogglePause();

    Core core;

private:
    void HandleInput();
    void RenderMenubar();
    void Boot(const std::string& path);
    void RenderDisplayWindow();
    void render_library_window();
    
    void BeginFullscreenWindow(const char *name, ImVec2 padding = ImVec2(0.0f, 0.0f));
    void EndFullscreenWindow();

    void render_debugger_window();
    void render_ee_debugger();
    void render_iop_debugger();

    const char* glsl_version = "#version 330";

    SDL_Window* window;
    SDL_GLContext gl_context;
    
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
    bool running = true;
    ImGui::FileBrowser file_dialog;
    
    int window_width;
    int window_height;

    static constexpr int menubar_height = 18;
    GLuint screen_texture;

    enum class WindowState {
        Library,
        Display,
    };

    WindowState window_state = WindowState::Library;
    common::GamesList games_list;

    float fps;
    bool m_show_debugger_window{true};
    bool m_show_demo_window{false};
    int m_ee_disassembly_size{15};
    int m_iop_disassembly_size{15};
};
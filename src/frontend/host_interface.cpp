#include "frontend/host_interface.h"

HostInterface::HostInterface() :
    core([this](float fps) {
        UpdateTitle(fps);
    }) {
}

bool HostInterface::Initialise() {
    // initialise sdl
    if (SDL_Init(SDL_INIT_VIDEO) > 0) {
        common::Warn("error initialising SDL");
        return false;
    }

    // decide gl + glsl versions
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window = SDL_CreateWindow("matcha", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // setup imgui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // setup sdl and opengl3 backend
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    io.Fonts->AddFontFromFileTTF("../data/fonts/roboto-regular.ttf", 13.0f);
    io.Fonts->AddFontFromFileTTF("/../data/fonts/Consolas.ttf", 14.0f);
    SetupStyle();

    return true;
}

void HostInterface::Run() {
    while (running) {
        // poll events
        HandleInput();

        // start imgui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        RenderMenubar();

        // show demo window
        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        if (ee_debugger.show_registers_window) {
            ee_debugger.RegistersWindow(core.system.ee);
        }

        if (ee_debugger.show_disassembly_window) {
            ee_debugger.DisassemblyWindow(core);
        }

        if (iop_debugger.show_registers_window) {
            iop_debugger.RegistersWindow(*core.system.iop_core);
        }

        if (iop_debugger.show_disassembly_window) {
            iop_debugger.DisassemblyWindow(core);
        }

        // rendering
        ImGui::Render();
        glViewport(0, 0, 1280, 720);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }
}

void HostInterface::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void HostInterface::HandleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
            running = false;
        }
    }
}

void HostInterface::RenderMenubar() {
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 6.0f));
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load ROM")) {
                file_dialog.Open();
            }

            if (ImGui::MenuItem("Boot BIOS")) {
                Boot("");
            }

            if (ImGui::MenuItem("Quit")) {
                running = false;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Emulator")) {
            if (ImGui::MenuItem(core.GetState() == CoreState::Running ? "Pause" : "Resume")) {
                TogglePause();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debugger")) {
            if (ImGui::BeginMenu("EE")) {
                ImGui::MenuItem("Registers", nullptr, &ee_debugger.show_registers_window);
                ImGui::MenuItem("Disassembly", nullptr, &ee_debugger.show_disassembly_window);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("IOP")) {
                ImGui::MenuItem("Registers", nullptr, &iop_debugger.show_registers_window);
                ImGui::MenuItem("Disassembly", nullptr, &iop_debugger.show_disassembly_window);
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    ImGui::PopStyleVar(2);

    file_dialog.Display();
    if (file_dialog.HasSelected()) {
        Boot(file_dialog.GetSelected().string());
        file_dialog.ClearSelected();
    }
}

void HostInterface::SetupStyle() {
    ImGui::GetStyle().WindowBorderSize = 1.0f;
    ImGui::GetStyle().PopupBorderSize = 0.0f;
    ImGui::GetStyle().ChildBorderSize = 0.0f;
    ImGui::GetStyle().FrameBorderSize = 1.0f;
    ImGui::GetStyle().GrabMinSize = 7.0f;
    ImGui::GetStyle().WindowRounding = 4.0f;
    ImGui::GetStyle().FrameRounding = 1.0f;
    ImGui::GetStyle().PopupRounding = 0.0f;
    ImGui::GetStyle().ChildRounding = 0.0f;
    ImGui::GetStyle().GrabRounding = 4.0f;
    ImGui::GetStyle().TabRounding = 0.0f;
    ImGui::GetStyle().ScrollbarSize = 10.0f;
    ImGui::GetStyle().ScrollbarRounding = 12.0f;
    ImGui::GetStyle().WindowTitleAlign = ImVec2(0.50f, 0.50f);
    ImGui::GetStyle().WindowMenuButtonPosition = ImGuiDir_None;
    ImGui::GetStyle().FramePadding = ImVec2(4.0f, 2.0f);
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = colour_darker_grey;
    ImGui::GetStyle().Colors[ImGuiCol_Text] = colour_white;
    ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = colour_black;
    ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = colour_black;
    ImGui::GetStyle().Colors[ImGuiCol_Header] = colour_grey;
    ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered] = colour_lighter_lighter_lighter_lighter_grey;
    ImGui::GetStyle().Colors[ImGuiCol_HeaderActive] = colour_lighter_lighter_lighter_lighter_grey;
    ImGui::GetStyle().Colors[ImGuiCol_ResizeGrip] = colour_grey;
    ImGui::GetStyle().Colors[ImGuiCol_ResizeGripHovered] = colour_blue;
    ImGui::GetStyle().Colors[ImGuiCol_ResizeGripActive] = colour_blue;
    ImGui::GetStyle().Colors[ImGuiCol_Border] = colour_lighter_grey;
    ImGui::GetStyle().Colors[ImGuiCol_Button] = colour_grey;
    ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] = colour_lighter_grey;
    ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] = colour_lighter_lighter_grey;
    ImGui::GetStyle().Colors[ImGuiCol_Tab] = colour_black;
    ImGui::GetStyle().Colors[ImGuiCol_TabUnfocused] = colour_black;
    ImGui::GetStyle().Colors[ImGuiCol_TabUnfocusedActive] = colour_black;
    ImGui::GetStyle().Colors[ImGuiCol_TabHovered] = colour_blue;
    ImGui::GetStyle().Colors[ImGuiCol_TabActive] = colour_blue;
    ImGui::GetStyle().Colors[ImGuiCol_CheckMark] = colour_blue;
    ImGui::GetStyle().Colors[ImGuiCol_SliderGrab] = colour_blue;
    ImGui::GetStyle().Colors[ImGuiCol_SliderGrabActive] = colour_blue;
    ImGui::GetStyle().Colors[ImGuiCol_Separator] = colour_lighter_grey;
    ImGui::GetStyle().Colors[ImGuiCol_SeparatorHovered] = colour_clearer_blue;
    ImGui::GetStyle().Colors[ImGuiCol_SeparatorActive] = colour_blue;
    ImGui::GetStyle().Colors[ImGuiCol_FrameBg] = colour_grey;
    ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered] = colour_lighter_grey;
    ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive] = colour_lighter_lighter_grey;
    ImGui::GetStyle().Colors[ImGuiCol_TableBorderStrong] = colour_black;
    ImGui::GetStyle().Colors[ImGuiCol_MenuBarBg] = colour_lighter_grey;
    ImGui::GetStyle().Colors[ImGuiCol_ChildBg] = colour_darker_grey;
    ImGui::GetStyle().Colors[ImGuiCol_PopupBg] = colour_black;
    ImGui::GetStyle().Colors[ImGuiCol_ScrollbarBg] = colour_black;
    // ImGui::GetStyle().Colors[ImGuiCol_DockingPreview] = colour_blue;
    // ImGui::GetStyle().Colors[ImGuiCol_DockingEmptyBg] = colour_black;
}

void HostInterface::UpdateTitle(float fps) {
    char window_title[100];
    float percent_usage = (fps / 60.0f) * 100;
    snprintf(window_title, 100, "otterstation | %0.2f FPS | %0.2f%s", fps, percent_usage, "%");
    SDL_SetWindowTitle(window, window_title);
}

void HostInterface::TogglePause() {
    if (core.GetState() == CoreState::Running) {
        core.SetState(CoreState::Paused);
    } else {
        core.SetState(CoreState::Running);
    }
}

void HostInterface::Boot(const std::string& path = "") {
    core.Reset();
    core.SetGamePath(path);
    core.SetState(CoreState::Running);
}
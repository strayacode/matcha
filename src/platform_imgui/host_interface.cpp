#include "host_interface.h"

HostInterface::HostInterface() :
    core([this](float fps) {
        UpdateTitle(fps);
    }) {
}

bool HostInterface::Initialise() {
    // initialise sdl
    if (SDL_Init(SDL_INIT_VIDEO) > 0) {
        log_warn("error initialising SDL");
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
    window = SDL_CreateWindow("otterstation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
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
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load ROM")) {
                file_dialog.Open();
            }

            if (ImGui::MenuItem("Quit")) {
                running = false;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    file_dialog.Display();
    if (file_dialog.HasSelected()) {
        core.Reset();
        core.SetGamePath(file_dialog.GetSelected().string());
        core.SetState(CoreState::Running);
        file_dialog.ClearSelected();
    }
}

void HostInterface::SetupStyle() {
    ImGui::GetStyle().WindowBorderSize = 0.0f;
    ImGui::GetStyle().PopupBorderSize = 0.0f;
    ImGui::GetStyle().ChildBorderSize = 0.0f;
    ImGui::GetStyle().WindowRounding = 5.0f;
    ImGui::GetStyle().FrameRounding = 0.0f;
    ImGui::GetStyle().PopupRounding = 0.0f;
    ImGui::GetStyle().ChildRounding = 0.0f;
    ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImVec4(0.059f, 0.059f, 0.059f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = ImVec4(0.059f, 0.059f, 0.059f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_Header] = ImVec4(0.140f, 0.140f, 0.140f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered] = ImVec4(0.160f, 0.273f, 0.632f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_HeaderActive] = ImVec4(0.160f, 0.273f, 0.632f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_ResizeGrip] = ImVec4(0.140f, 0.140f, 0.140f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.160f, 0.273f, 0.632f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.160f, 0.273f, 0.632f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_Button] = ImVec4(0.140f, 0.140f, 0.140f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] = ImVec4(0.160f, 0.273f, 0.632f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] = ImVec4(0.349f, 0.500f, 0.910f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_Tab] = ImVec4(0.140f, 0.140f, 0.140f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_TabHovered] = ImVec4(0.160f, 0.273f, 0.632f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_TabActive] = ImVec4(0.160f, 0.273f, 0.632f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBg] = ImVec4(0.140f, 0.140f, 0.140f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.160f, 0.273f, 0.632f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive] = ImVec4(0.160f, 0.273f, 0.632f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.114f, 0.114f, 0.114f, 1.000f);
}

void HostInterface::UpdateTitle(float fps) {
    char window_title[100];
    float percent_usage = (fps / 60.0f) * 100;
    snprintf(window_title, 100, "otterstation | %0.2f FPS | %0.2f%s", fps, percent_usage, "%");
    SDL_SetWindowTitle(window, window_title);
}
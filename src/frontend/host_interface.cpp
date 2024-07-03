#include "common/string.h"
#include "frontend/host_interface.h"
#include "imgui/imgui_internal.h"

HostInterface::HostInterface() :
    core([this](float fps) {
        this->fps = fps;
    }) {
}

bool HostInterface::initialise() {
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
    // SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    window = SDL_CreateWindow(
        "matcha",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280,
        720,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    SDL_GetWindowSize(window, &window_width, &window_height);

    // setup imgui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // setup sdl and opengl3 backend
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // initialise texture stuff
    glGenTextures(1, &screen_texture);
    glBindTexture(GL_TEXTURE_2D, screen_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    games_list.Initialise();

    return true;
}

void HostInterface::run() {
    while (running) {
        // poll events
        HandleInput();

        // start imgui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        RenderMenubar();

        switch (window_state) {
        case WindowState::Library:
            // RenderLibraryWindow();
            break;
        case WindowState::Display:
            RenderDisplayWindow();
            break;
        }

        if (m_show_debugger_window) {
            render_debugger_window();
        }

        if (m_show_demo_window) {
            ImGui::ShowDemoWindow(&m_show_demo_window);
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

void HostInterface::shutdown() {
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
                core.SetBootParameters(BootMode::BIOS);
                core.Boot();
                window_state = WindowState::Display;
            }

            if (ImGui::MenuItem("Power Off")) {
                window_state = WindowState::Library;
                core.SetState(CoreState::Idle);
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

            if (ImGui::MenuItem("Restart")) {
                if (core.GetState() != CoreState::Idle) {
                    core.Boot();
                }
            }

            ImGui::EndMenu();
        }

        ImGui::MenuItem("Debugger", nullptr, &m_show_debugger_window);
        ImGui::MenuItem("Demo", nullptr, &m_show_demo_window);

        if (core.GetState() == CoreState::Running && fps != 0.0f) {
            std::string fps_string = common::Format("%.0f FPS | %.2f ms", fps, 1000.0f / fps);
            auto pos = window_width - ImGui::CalcTextSize(fps_string.c_str()).x - ImGui::GetStyle().ItemSpacing.x;

            ImGui::SetCursorPosX(pos);
            ImGui::Text("%s", fps_string.c_str());
        } else if (core.GetState() == CoreState::Paused) {
            std::string fps_string = "Paused";

            auto pos = window_width - ImGui::CalcTextSize(fps_string.c_str()).x - ImGui::GetStyle().ItemSpacing.x;

            ImGui::SetCursorPosX(pos);
            ImGui::Text("%s", fps_string.c_str());
        }

        ImGui::EndMainMenuBar();
    }

    ImGui::PopStyleVar(2);

    file_dialog.Display();
    if (file_dialog.HasSelected()) {
        core.SetBootParameters(BootMode::Fast, file_dialog.GetSelected().string());
        core.Boot();
        window_state = WindowState::Display;
        file_dialog.ClearSelected();
    }
}

void HostInterface::TogglePause() {
    if (core.GetState() == CoreState::Running) {
        core.SetState(CoreState::Paused);
    } else if (core.GetState() == CoreState::Paused) {
        core.SetState(CoreState::Running);
    }
}

void HostInterface::RenderDisplayWindow() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

    gs::Framebuffer framebuffer = core.system.gs.GetFramebuffer();

    glBindTexture(GL_TEXTURE_2D, screen_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, framebuffer.width, framebuffer.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, framebuffer.data);

    const double scale_x = static_cast<double>(window_width) / framebuffer.width;
    const double scale_y = static_cast<double>(window_height - menubar_height) / framebuffer.height;
    const double scale = scale_x < scale_y ? scale_x : scale_y;

    ImVec2 scaled_dimensions = ImVec2(framebuffer.width * scale, framebuffer.height * scale);
    ImVec2 center_pos = ImVec2(
        (static_cast<double>(window_width) - scaled_dimensions.x) / 2,
        (static_cast<double>(window_height - menubar_height) - scaled_dimensions.y) / 2
    );
  
    ImGui::GetBackgroundDrawList()->AddImage(
        (void*)(intptr_t)screen_texture,
        ImVec2(center_pos.x, menubar_height + center_pos.y),
        ImVec2(center_pos.x + scaled_dimensions.x, menubar_height + center_pos.y + scaled_dimensions.y),
        ImVec2(0, 0),
        ImVec2(1, 1),
        IM_COL32_WHITE
    );

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
}

void HostInterface::RenderLibraryWindow() {
    BeginFullscreenWindow("Library");
    static ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable
        | ImGuiTableFlags_RowBg
        | ImGuiTableFlags_BordersOuterV
        | ImGuiTableFlags_SizingStretchProp;

    float min_row_height = 20.0f;

    if (ImGui::BeginTable("Library", 3, flags)) {
        ImGui::TableSetupColumn("Title");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Size");
        ImGui::TableHeadersRow();

        int row = 0;
        for (const common::GamesList::Entry& entry : games_list.GetEntries()) {
            ImGui::TableNextRow(ImGuiTableRowFlags_None);
            ImGui::PushID(row);
            ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.0f, 0.5f));
            ImGui::TableSetColumnIndex(0);

            if (ImGui::Selectable(entry.name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap, ImVec2(0.0f, min_row_height))) {
                core.SetBootParameters(BootMode::Fast, entry.path);
                core.Boot();
                window_state = WindowState::Display;
            }

            ImGui::TableSetColumnIndex(1);

            if (ImGui::Selectable(entry.type.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap, ImVec2(0.0f, min_row_height))) {
                core.SetBootParameters(BootMode::Fast, entry.path);
                core.Boot();
                window_state = WindowState::Display;
            }

            ImGui::TableSetColumnIndex(2);

            if (ImGui::Selectable(entry.size.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap, ImVec2(0.0f, min_row_height))) {
                core.SetBootParameters(BootMode::Fast, entry.path);
                core.Boot();
                window_state = WindowState::Display;
            }

            ImGui::PopStyleVar();
            ImGui::PopID();
            row++;
        }

        ImGui::EndTable();
    }

    EndFullscreenWindow();
}

void HostInterface::BeginFullscreenWindow(const char *name, ImVec2 padding) {
    ImGui::SetNextWindowPos(ImVec2(0, menubar_height));
    ImGui::SetNextWindowSize(ImVec2(window_width, window_height - menubar_height));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin(
        name,
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoBringToFrontOnFocus
    );
}

void HostInterface::EndFullscreenWindow() {
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
}
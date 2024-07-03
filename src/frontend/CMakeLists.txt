set(SOURCES
    imgui/imconfig.h
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imfilebrowser.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
    imgui/imgui_impl_opengl3.cpp
    imgui/imgui_impl_opengl3.h
    imgui/imgui_impl_sdl2.cpp
    imgui/imgui_impl_sdl2.h
    main.cpp
    host_interface.cpp
    debugger.cpp
)

add_executable(matcha ${SOURCES})

find_package(SDL2 REQUIRED)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)
target_link_libraries(matcha ${OPENGL_LIBRARIES})

include_directories(imgui ${SDL2_INCLUDE_DIRS})
target_link_libraries(matcha core common ${SDL2_LIBRARIES})

find_package(Threads REQUIRED)

if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    find_package(X11 REQUIRED)
endif()

target_link_libraries(matcha ${CMAKE_THREAD_LIBS_INIT} ${X11_LIBRARIES} ${CMAKE_DL_LIBS})
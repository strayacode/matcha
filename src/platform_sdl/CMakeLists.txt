add_executable(otterstation main.cpp host_interface.cpp host_interface.h)

find_package(SDL2 REQUIRED)

include_directories(otterstation ${SDL2_INCLUDE_DIRS})

target_link_libraries(otterstation PRIVATE core common ${SDL2_LIBRARIES})

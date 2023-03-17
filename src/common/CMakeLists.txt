add_library(common
    types.h
    log.h log.cpp
    emu_thread.h emu_thread.cpp
    bits.h bits.cpp
    queue.h
    memory.h virtual_page_table.h
    string.h string.cpp
    filesystem.h filesystem.cpp
    games_list.h games_list.cpp
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(common PRIVATE Threads::Threads)

set_target_properties(common PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
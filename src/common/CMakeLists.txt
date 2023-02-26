add_library(common
    types.h
    log.h log.cpp
    memory_helpers.h
    emu_thread.h emu_thread.cpp
    arithmetic.h arithmetic.cpp
    cpu_types.h ring_buffer.h
    memory.h virtual_page_table.h
    string.h
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(common PRIVATE Threads::Threads)

set_target_properties(common PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
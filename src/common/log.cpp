#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include "common/log.h"

constexpr static bool debug = true;

#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define RESET "\x1B[0m"

namespace common {
    void info(const char* format, ...) {
        std::va_list args;
        va_start(args, format);
        std::printf("[" GREEN "INFO" RESET "] ");
        std::vfprintf(stdout, format, args);
        std::printf("\n");
        std::fflush(stdout);
        va_end(args);
    }

    void debug(const char* format, ...) {
        std::va_list args;
        va_start(args, format);
        std::printf("[" BLUE "DEBUG" RESET "] ");
        std::vfprintf(stdout, format, args);
        std::printf("\n");
        std::fflush(stdout);
        va_end(args);
    }

    void warn(const char* format, ...) {
        std::va_list args;
        va_start(args, format);
        std::printf("[" YELLOW "WARN" RESET "] ");
        std::vfprintf(stdout, format, args);
        std::printf("\n");
        std::fflush(stdout);
        va_end(args);
    }

    void error(const char* format, ...) {
        std::va_list args;
        va_start(args, format);
        std::printf("[" RED "ERROR" RESET "] ");
        std::vfprintf(stderr, format, args);
        std::printf("\n");
        std::fflush(stderr);
        va_end(args);
        std::exit(0);
    }
}
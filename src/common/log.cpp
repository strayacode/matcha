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

FILE* fp = fopen("matcha.log", "w");

namespace common {
    
void Info(const char* format, ...) {
    std::va_list args;
    va_start(args, format);
    std::printf("[" GREEN "INFO" RESET "] ");
    std::vfprintf(stdout, format, args);
    std::printf("\n");
    std::fflush(stdout);
    va_end(args);
}

void Debug(const char* format, ...) {
    std::va_list args;
    va_start(args, format);
    std::printf("[" BLUE "DEBUG" RESET "] ");
    std::vfprintf(stdout, format, args);
    std::printf("\n");
    std::fflush(stdout);
    va_end(args);
}

void Warn(const char* format, ...) {
    std::va_list args;
    va_start(args, format);
    std::printf("[" YELLOW "WARN" RESET "] ");
    std::vfprintf(stdout, format, args);
    std::printf("\n");
    std::fflush(stdout);
    va_end(args);
}

void Error(const char* format, ...) {
    std::va_list args;
    va_start(args, format);
    std::fprintf(stderr, "[" RED "ERROR" RESET "] ");
    std::vfprintf(stderr, format, args);
    std::fprintf(stderr, "\n");
    std::fflush(stderr);
    va_end(args);
    std::exit(0);
}

void Log(const char* format, ...) {
    std::va_list args;
    va_start(args, format);
    std::vfprintf(fp, format, args);
    std::fprintf(fp, "\n");
    va_end(args);
}

void LogNoNewline(const char* format, ...) {
    std::va_list args;
    va_start(args, format);
    std::vfprintf(fp, format, args);
    va_end(args);
}

} // namespace common
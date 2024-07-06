#include <cstdio>
#include <cstdlib>

namespace common {

void Info(const char* format, ...);
void Debug(const char* format, ...);
void Warn(const char* format, ...);
void Error(const char* format, ...);
void Log(const char* format, ...);
void LogNoNewline(const char* format, ...);

} // namespace common

#define LOG_TODO(fmt, ...) do {\
    fprintf(stderr, "%s:%d: " fmt "\n", __FILE__, __LINE__, __VA_ARGS__);\
    exit(1);\
} while (0)

#define LOG_TODO_NO_ARGS(fmt, ...) do {\
    fprintf(stderr, "%s:%d: " fmt "\n", __FILE__, __LINE__);\
    exit(1);\
} while (0)
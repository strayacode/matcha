#pragma once

#include <stdarg.h>
#include <stdio.h>

class LogFile {
public:
    LogFile(const LogFile& log_file) = delete;

    ~LogFile() {
        fclose(fp);
    }

    static LogFile& Get() {
        return instance;
    }

    void Log(const char *format, ...) {
        va_list args;

        va_start(args, format);
        vfprintf(fp, format, args);
        va_end(args);
    }

private:
    LogFile() {};

    FILE* fp = fopen("../../log-stuff/otterstation.log", "w");
    static LogFile instance;
};
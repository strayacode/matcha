#pragma once

#include <stdarg.h>
#include <stdio.h>
#include "log.h"

class LogFile {
public:
    ~LogFile() {
        fclose(fp);
    }

    void SetPath(const char *path) {
        fp = fopen(path, "w");

        if (fp == NULL) {
            log_fatal("[LogFile] Path %s doesn't exist yet", path);
        }
    }

    void Log(const char *format, ...) {
        va_list args;

        va_start(args, format);
        vfprintf(fp, format, args);
        va_end(args);
    }

private:
    FILE* fp;
};
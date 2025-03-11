#ifndef LOGGER_H
#define LOGGER_H

#include <ncurses.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include "defs.h"

// Logging macros
#define LOG_INFO(...) LogMessage(LOG_INFO, __VA_ARGS__)
#define LOG_WARNING(...) LogMessage(LOG_WARNING, __VA_ARGS__)
#define LOG_ERROR(...) LogMessage(LOG_ERROR, __VA_ARGS__)

extern WINDOW *console_win;  // Global console window
extern bool console_enabled; // Flag to enable or disable console

/// @brief LogLevel, note that the value of the enums coorespond to its color
typedef enum {
    LOG_INFO = 0,
    LOG_WARNING = 3,
    LOG_ERROR = 1
} LogLevel;

void LogMessage(LogLevel level, const char *format, ...);

#endif

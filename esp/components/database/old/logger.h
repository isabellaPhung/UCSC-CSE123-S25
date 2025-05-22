#ifndef LOGGER_H
#define LOGGER_H

#ifdef ESP_PLATFORM
#include "esp_log.h"

// This is defined when building under ESP-IDF for any ESP chip
// IMPORTANT: ASSUMING THERE IS A GLOBAL VARIABLE CALLED TAG WITH LIBRARY NAME
#define LOG_INFO(...) ESP_LOGI(TAG, __VA_ARGS__)
#define LOG_WARNING(...) ESP_LOGW(TAG, __VA_ARGS__)
#define LOG_ERROR(...) ESP_LOGE(TAG, __VA_ARGS__)

#else
// Use standard printf for desktop usage
#include <stdarg.h> // To process multiple inputs
#include <stdio.h>

#define RESET "\033[0m"
#define YELLOW "\033[33m"
#define RED "\033[31m"
#define CYAN "\033[36m"

// Logging macros
#define LOG_INFO(...) LogMessage(LOG_INFO, __VA_ARGS__)
#define LOG_WARNING(...) LogMessage(LOG_WARNING, __VA_ARGS__)
#define LOG_ERROR(...) LogMessage(LOG_ERROR, __VA_ARGS__)

/// @brief LogLevel
typedef enum
{
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

void LogMessage(LogLevel level, const char *format, ...)
{
    char buffer[256]; // Don't write super long error messages please
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    switch (level)
    {
    case LOG_INFO:
        printf("INFO: %s\n", buffer);
        break;
    case LOG_WARNING:
        printf(YELLOW "WARNING: %s\n" RESET, buffer);
        break;
    case LOG_ERROR:
        printf(RED "ERROR: %s\n" RESET, buffer);
        break;
    }
}

#endif
#endif
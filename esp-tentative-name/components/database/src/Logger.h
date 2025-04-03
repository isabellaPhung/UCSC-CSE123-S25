#ifndef LOGGER_H
#define LOGGER_H

// Logging macros
#define LOG_INFO(...) LogMessage(LOG_INFO, __VA_ARGS__)
#define LOG_WARNING(...) LogMessage(LOG_WARNING, __VA_ARGS__)
#define LOG_ERROR(...) LogMessage(LOG_ERROR, __VA_ARGS__)

/// @brief LogLevel
typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

void LogMessage(LogLevel level, const char *format, ...);

#endif
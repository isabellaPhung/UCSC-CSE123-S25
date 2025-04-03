// Logger for debugging purposes, to be ran in terminal
#include "Logger.h"

#include <stdarg.h>	// To process multiple inputs
#include <stdio.h>

#define RESET  "\033[0m"
#define YELLOW "\033[33m"
#define RED    "\033[31m"
#define CYAN   "\033[36m"

void LogMessage(LogLevel level, const char *format, ...)
{
	char buffer[256];	// Don't write super long error messages please
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
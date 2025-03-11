#include "Logger.h"
#include "string.h"

WINDOW *console_win = NULL;   // Initialize the global console window
bool console_enabled = false; // Initialize the console enabled flag

static char log_buffer[CONSOLE_LINES][CONSOLE_WIDTH];
static LogLevel log_buffer_level[CONSOLE_LINES];
static int current_line = 0;

void LogMessage(LogLevel level, const char *format, ...)
{
    if (!console_enabled || console_win == NULL)
    {
        return; // If the console is not enabled or not initialized, return
    }

    va_list args;
    va_start(args, format);

    char buffer[CONSOLE_WIDTH - 9];
    vsnprintf(buffer, sizeof(buffer), format, args);

    // Store the message in the circular buffer
    switch (level)
    {
    case LOG_INFO:
        snprintf(log_buffer[current_line], CONSOLE_WIDTH, "INFO: %s", buffer);
        break;
    case LOG_WARNING:
        snprintf(log_buffer[current_line], CONSOLE_WIDTH, "WARNING: %s", buffer);
        break;
    case LOG_ERROR:
        snprintf(log_buffer[current_line], CONSOLE_WIDTH, "ERROR: %s", buffer);
        break;
    }
    log_buffer_level[current_line] = level;

    // Increment the current line index and wrap around if necessary
    current_line = (current_line + 1) % CONSOLE_LINES;

    // Clear the console window and draw the box
    wclear(console_win);

    // Display the log messages from the circular buffer
    for (int i = 0; i < CONSOLE_LINES; i++)
    {
        int line_index = (current_line + i) % CONSOLE_LINES;
        if (strlen(log_buffer[line_index]) > 0)
        {
            wattron(console_win, COLOR_PAIR((int)log_buffer_level[line_index]));
            mvwprintw(console_win, i, 0, "%s", log_buffer[line_index]);
            wattroff(console_win, COLOR_PAIR((int)log_buffer_level[line_index]));
        }
    }

    wrefresh(console_win);
    va_end(args);
}

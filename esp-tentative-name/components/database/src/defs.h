// Define console size
#ifndef DEFS_H
#define DEFS_H

#define FOCUS_WIDTH 30
#define MENU_WIDTH 60
#define CONSOLE_WIDTH 93

// Define window sizes
#define FOCUS_LINES 21
#define MENU_LINES 21
#define CONSOLE_LINES 9

#define DESC_SIZE FOCUS_WIDTH * (FOCUS_LINES - 5) + 1 // Description size
#define NAME_SIZE FOCUS_WIDTH + 1   // Focus menu must be able to show full name

#define SQL_MESSAGE_SIZE 124 + NAME_SIZE + 23 + 23 + 1 + 1 + DESC_SIZE

#define SQL_EAT_COMPLETED FALSE  // Included completed entries

#endif
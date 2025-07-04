#ifndef DEFS_H
#define DEFS_H

// SPI Initialization
#define SPI_HOST SPI2_HOST

#define SQLITE_MEMORY_SIZE 52 * 1024

#define MAX_NAME_SIZE 128
#define MAX_DESC_SIZE 1024

/// Priority is an offset parameter, this value decides how many seconds difference each point
/// of priority is worth.
/// Set to a quarter of a day, such that a task with a priority of 4 would be seen as having a due
/// date of 1 day sooner by the sorting algorithm.
#define PRIORITY_MULTIPLIER "21600"

#define UUID_LENGTH 37 // 36 characters + null terminator

#define MOUNT_POINT "/sdcard"
#define MESSAGE_BUFFER_NAME MOUNT_POINT "/responses.json"

// Habit Flags
#define SATURDAY_FLAG 0x01
#define FRIDAY_FLAG 0x02
#define THURSDAY_FLAG 0x04
#define WEDNESDAY_FLAG 0x08
#define TUESDAY_FLAG 0x10
#define MONDAY_FLAG 0x20
#define SUNDAY_FLAG 0x40

#endif
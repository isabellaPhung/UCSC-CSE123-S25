#ifndef DEFS_H
#define DEFS_H

#define MAX_NAME_SIZE 128
#define MAX_DESC_SIZE 1024

/// Priority is an offset parameter, this value decides how many seconds difference each point
/// of priority is worth.
/// Set to a quarter of a day, such that a task with a priority of 4 would be seen as having a due 
/// date of 1 day sooner by the sorting algorithm.
#define PRIORITY_MULTIPLIER "21600"

#define UUID_LENGTH 37 // 36 characters + null terminator

#define MOUNT_POINT "/sdcard"

#endif
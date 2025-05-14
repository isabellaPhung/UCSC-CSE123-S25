#ifndef HABIT_H
#define HABIT_H

#include <sqlite3.h>
#include <time.h>
#include <cJSON.h>

#include "defs.h"

#include "esp_err.h"

typedef struct
{
    char uuid[UUID_LENGTH];
    char name[MAX_NAME_SIZE];
    uint8_t goal;
} habit_t;

int RetrieveHabitsDB(sqlite3 *db, habit_t *habitBuffer, int count, int offset);

/// @brief Adds habit entry from JSON script to create a new habit in database
esp_err_t ParseHabitsJSON(sqlite3 *db, const cJSON *habit);

/// @brief Create new Habit Table (type)
/// @param uuid Unique idea for table
/// @param name Name of table
/// @param goal_flag Flags for which day of the week the task should be compelted
esp_err_t HabitAddDB(sqlite3 *db, const char *uuid, const char *name, uint8_t goal_flags);

/// @brief Appends a new entry to the database, with the type defined by the uuid of the habit table
/// @param habid_id The forign key of the entry defining what list the task is appended to
/// @param date ISO 8601 formatted date accuracy up to day (Ex: 2025-05-05)
esp_err_t HabitAddEntryDB(sqlite3 *db, const char *habit_id, time_t datetime);

/// @brief Removes a new entry if valid
/// @returns Error code. ESP_OK dictates that the entry doesn't exist, not whether an entry was
/// removed
esp_err_t HabitRemoveEntryDB(sqlite3 *db, const char *habit_id, time_t datetime);

// --------------------------------- Check if Habit is Relavent -----------------------------------

/// @brief Determines if an entry exists on the inculded date
/// @returns 1 on found, 0 on not found, -1 on error
int HabitEntryCompletedDB(sqlite3 *db, const char *habit_id, time_t datetime);

/// @brief Checks if the user needs to complete a task on a weekday
/// @param wday Day of the week starting on Sunday (0-6)
///             Defined in tm struct
int HabitEntryDueDB(sqlite3 *db, const char *habit_id, time_t datetime);

// DEBUG FUNCTION
//esp_err_t TestHabitFunctions(sqlite3 *db);

#endif
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

    // Status of the last 7 days; 0 on incomplete, 1 on complete, 2 on due
    int completed[7];
} habit_t;

int RetrieveHabitsDB(habit_t *habitBuffer, int count, int offset);

/// @brief Adds habit entry from JSON script to create a new habit in database
esp_err_t ParseHabitsJSON(cJSON *habit);

/// @brief Create new Habit Table (type)
/// @param uuid Unique idea for table
/// @param name Name of table
/// @param goal_flag Flags for which day of the week the task should be compelted
esp_err_t HabitAddDB(const char *uuid, const char *name, uint8_t goal_flags);

/// @brief Appends a new entry to the database, with the type defined by the uuid of the habit table
/// @param habid_id The forign key of the entry defining what list the task is appended to
/// @param date ISO 8601 formatted date accuracy up to day (Ex: 2025-05-05)
esp_err_t HabitAddEntryDB(const char *habit_id, time_t datetime);

/// @brief Removes a new entry if valid
/// @returns Error code. ESP_OK dictates that the entry doesn't exist, not whether an entry was
/// removed
esp_err_t HabitRemoveEntryDB(const char *habit_id, time_t datetime);

// --------------------------------- Check if Habit is Relavent -----------------------------------

/// @brief Configures a habit's completion array with not complete (0), completed (1), and to-do (2) flags
///        Inserted based on relative date from today (i.e. completed[4] is the status of the day 4 days ago)
/// @param db Database to retrieve entry information from
/// @param habit A habit that was sent through RetrieveHabitsDB
/// @param date Date to reference from
esp_err_t HabitRetrieveWeekCompletionDB(habit_t *habit, time_t date);

#endif
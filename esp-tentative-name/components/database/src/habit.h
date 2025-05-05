#ifndef HABIT_H
#define HABIT_H

#include <sqlite3.h>
#include <time.h>

#include "defs.h"

#include "esp_err.h"

/// @brief Create new Habit Table (type)
/// @param uuid Unique idea for table
/// @param name Name of table
esp_err_t HabitCreateDB(const char *uuid, const char *name);

/// @brief Appends a new entry to the database, with the type defined by the uuid of the habit table
/// @param habid_id The forign key of the entry defining what list the task is appended to
/// @param date ISO 8601 formatted date accuracy up to day (Ex: 2025-05-05)
esp_err_t HabitAddEntryDB(const char *habit_id, const char *date);

/// @brief Removes a new entry if valid
/// @returns Error code. ESP_OK dictates that the entry doesn't exist, not whether an entry was
/// removed
esp_err_t HabitRemoveEntryDB(const char *habit_id, const char *date);

/// @brief Determines if an entry exists on the inculded date
/// @returns 1 on found, 0 on not found, -1 on error
int HabitCheckEntryDB(const char *habit_id, const char *date);

#endif
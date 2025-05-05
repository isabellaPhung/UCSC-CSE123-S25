#ifndef TASK_H
#define TASK_H

#include <sqlite3.h>
#include <time.h>
#include "esp_err.h"

#include "defs.h"

typedef enum
{
    INCOMPLETE = 0,
    COMPLETE = 1,
    MFD = 2 // Marked For Deletion
} TASK_STATUS;

/** Task Struct
 * UUID string used for id to match with JSON format.
 */
typedef struct
{
    char uuid[UUID_LENGTH];               // UUID string of entry
    char name[MAX_NAME_SIZE];        // Title of entry
    time_t time;                          // Unix time (UTC)
    char priority;                        // Priority 0-9
    TASK_STATUS completion;               // Completion status of the entry
    char description[MAX_DESC_SIZE]; // Verbose description of entry
} task_t;

// --- Entry Creation ---

/// @brief Creates task entries based on JSON Script
/// @param db SQLite database object
/// @param json JSON text
/// @return Error code
int ParseTasksJSON(sqlite3 *db, const char *json);

// --- Entry modification ---

/// @brief Modifies the completion status of the task on the database or removes it from the 
///        database
/// @param db SQLite database object
/// @param uuid Task's unique ID
/// @param new_status New status. If this is MFD the object is deleted
/// @returns ESP Error Code
esp_err_t UpdateTaskStatusDB(sqlite3 *db, const char *uuid, TASK_STATUS new_status);

// --- Entry Recovery ---

/// @brief Retrieves a number of tasks sorted by date from the database with some offset from 
///        the most urgant task
/// @param db SQLite database object
/// @param taskBuffer Array of objects used to store the retrieved tasks
/// @param count Number of objects to store on the buffer
/// @param offset Offset
int RetrieveTasksSortedDB(sqlite3 *db, task_t *taskBuffer, int count, int offset);

// --- Utility ---
void PrintTask(task_t ent);
int PrintTaskDB(sqlite3 *db, const char *id);

#endif

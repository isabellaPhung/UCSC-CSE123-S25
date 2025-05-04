#ifndef TASK_H
#define TASK_H

#include <sqlite3.h>
#include <time.h>

#define MAX_TASK_NAME_SIZE 128
#define MAX_TASK_DESC_SIZE 1024

#define UUID_LENGTH 37 // 36 characters + null terminator

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
    char name[MAX_TASK_NAME_SIZE];        // Title of entry
    time_t time;                          // Unix time (UTC)
    char priority;                        // Priority 0-9
    TASK_STATUS completion;               // Completion status of the entry
    char description[MAX_TASK_DESC_SIZE]; // Verbose description of entry
} task_t;

int AddTaskDB(sqlite3 *db, task_t *ent);
int ParseTasksJSON(sqlite3 *db, const char *json);

// --- Entry modification ---
int RemoveTaskDB(sqlite3 *db, const char *id);
void CompleteTaskDB(sqlite3 *db, const char *id);

// --- Entry Recovery ---
int RetrieveTaskDB(sqlite3 *db, const char *id, task_t *ent);
int RetrieveTasksSortedDB(sqlite3 *db, task_t *taskMemory, int count);

// --- Utility ---
void PrintTask(task_t ent);
int PrintTaskDB(sqlite3 *db, const char *id);

#endif

#ifndef TASK_H
#define TASK_H

#include <sqlite3.h>
#include <time.h>

#define MAX_NAME 128
#define MAX_DESC 1024

typedef enum
{
    INCOMPLETE = 0,
    COMPLETE = 1,
    MFD = 2
} CompletionStatus;

typedef struct
{
    sqlite3_int64 id;            // id of entry within SQL
    char name[MAX_NAME];         // Title of entry
    time_t time;                 // Unit time (UTC)
    char priority;               // Priority 0-9
    CompletionStatus completion; // Completion status of the entry
    char description[MAX_DESC];  // Verbose description of entry
} Task;

// Adds a new entry to the tasks table
int AddEntry(sqlite3 *db,
             sqlite3_int64 *id,
             const char *name,
             time_t datetime,
             char priority,
             CompletionStatus completed,
             const char *description);

// Removes an entry from the tasks table based on the id
int RemoveEntry(sqlite3 *db,
                sqlite3_int64 id);

// Toggles the completion status of an entry
void CompleteEntry(sqlite3 *db,
                   sqlite3_int64 id);

// Prints entry info
int PrintEntry(sqlite3 *db,
               sqlite3_int64 id);

#endif
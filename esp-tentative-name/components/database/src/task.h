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
	MFD = 2 // Marked For Deletion
} CompletionStatus;

/** Task Struct
 * Since we want to know the packet size of our data all strings are explicetly sized.
 * 
 * Completion status has an additional bit dedicated for Marked For Deletion which should 
 * tell the server to archive the entry.
 */
typedef struct
{
	sqlite3_int64 id;			 // id of entry within SQL
	char name[MAX_NAME];		 // Title of entry
	time_t time;				 // Unit time (UTC)
	char priority;				 // Priority 0-9
	CompletionStatus completion; // Completion status of the entry
	char description[MAX_DESC];	 // Verbose description of entry
} Task;

/// @brief Adds an entry to the tasks table
/// @param db Database
/// @param id   Outputs the id of the entry
/// @param name Task name
/// @param datetim Datetime in Unix Timestamp format
/// @param priority The importance of the task (0-9)
/// @param completed    Whether the task is complete
/// @param description (optional) additional information about the task
/// @return SQLite Error
int AddEntry(sqlite3 *db,
			 sqlite3_int64 *id,
			 const char *name,
			 time_t datetime,
			 char priority,
			 CompletionStatus completed,
			 const char *description);

// --- Entry modification ---
/// @brief Deletes entry
/// @param db database to be altered
/// @param id entry id
/// @return SQLite Error
int RemoveEntry(sqlite3 *db,
				sqlite3_int64 id);

// Toggles the completion status of an entry
void CompleteEntry(sqlite3 *db,
				   sqlite3_int64 id);

// --- Entry Recovery ---

// Retrieve task entries
int RetrieveEntry(sqlite3 *db, sqlite3_int64 id, Task *ent);

/// @brief Retrieve entries sorted into a buffer
/// @param db sqlite database to pull from
/// @param taskMemory Buffer to place into. ASSUMING MEMORY IS ALLOCATED TO THE BUFFER.
/// @param count Maximum number of entries to be added. This should be set to the size of the buffer.
/// @return Number of entries returned, negative on error
int RetrieveEntriesSorted(sqlite3 *db, Task *taskMemory, int count); // TODO: Use a more dynamic buffer type like a Doubly Linked List

// --- Utility ---

/// @brief Prints task data to LOG_INFO
/// @param ent Task to have data printed of
void PrintTask(Task ent);

/// @brief Pulls task from database and prints task data to LOG_INFO
/// @param db
/// @param id
int PrintEntry(sqlite3 *db,
			   sqlite3_int64 id);

#endif
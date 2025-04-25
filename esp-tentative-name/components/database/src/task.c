#include "task.h"

#include <esp_log.h>
#include <stdio.h>
#include <string.h>

static const char *TAG = "task";

int AddTaskDB(sqlite3 *db, Task *ent)
{
	const char *sql;
	sqlite3_stmt *stmt;
	int rc;

	// Prepare the SQL statement based on the provided arguments
	sql = "INSERT INTO tasks (name, datetime, priority, completed, description) "
		  "VALUES (?, ?, ?, ?, ?);";

	// Prepare the SQL statement
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		ESP_LOGE(TAG, "Cannot prepare statement: %s", sqlite3_errmsg(db));
		return rc;
	}

	// Bind the values to the placeholders
	int index = 1;
	sqlite3_bind_text(stmt, index++, ent->name, -1, SQLITE_STATIC);

	sqlite3_bind_int64(stmt, index++, (sqlite3_int64)ent->time);
	sqlite3_bind_int(stmt, index++, (int)ent->priority);
	sqlite3_bind_int(stmt, index++, (int)ent->completion);

	sqlite3_bind_text(stmt, index++, ent->description, -1, SQLITE_STATIC);

	// Execute the statement
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		ESP_LOGE(TAG, "Execution failed: %s", sqlite3_errmsg(db));
		sqlite3_finalize(stmt);
		return rc;
	}

	// Clean up
	sqlite3_finalize(stmt);

	// Get the ID of the newly inserted row
	//sqlite3_int64 ID = sqlite3_last_insert_rowid(db);
	//*id = ID;

	ESP_LOGI(TAG, "SQL::AddEntry: Added %s", ent->name);

	return SQLITE_OK;
}

int RetrieveTaskDB(sqlite3 *db, const char* id, Task *ent)
{
	sqlite3_stmt *stmt;

	const char *sql = "SELECT * FROM tasks WHERE id = ?;";
	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
	sqlite3_bind_int64(stmt, 1, id);

	if (rc != SQLITE_OK)
	{
		ESP_LOGE(TAG, "SQL::RetrieveTaskDB: Failed to fetch data: %s", sqlite3_errmsg(db));
		return rc;
	}

	// ESP_LOGI(TAG, "SQL::RetrieveTaskDB: Retrieving tasks at id: %d", id);

	// Execute the statement and fetch rows
	while (sqlite3_step(stmt) != SQLITE_DONE)
	{
		const char *name = (const char *)sqlite3_column_text(stmt, 1);
		time_t time = (time_t)sqlite3_column_int64(stmt, 2);
		char priority = (char)sqlite3_column_int(stmt, 3);
		CompletionStatus completed = (CompletionStatus)sqlite3_column_int(stmt, 4);
		const char *description = (const char *)sqlite3_column_text(stmt, 5);

		strncpy(ent->id, id, UUID_SIZE);

		if (strlen(name) > MAX_TASK_NAME_SIZE)
		{
			ESP_LOGW(TAG, "Task::RetrieveTaskDB: Task Name is oversized, string will be truncated!");
		}
		strncpy(ent->name, name, MAX_TASK_NAME_SIZE);
		ent->time = time;
		ent->priority = priority;
		ent->completion = completed;
		if (description)
		{
			if (strlen(name) > MAX_TASK_NAME_SIZE)
			{
				ESP_LOGW(TAG, "Task::RetrieveTaskDB: Task Description is oversized, string will be truncated!");
			}
			strncpy(ent->description, description, MAX_TASK_DESC_SIZE);
		}
		else
		{
			ent->description[0] = '\0'; // Empty string
		}
	}

	sqlite3_finalize(stmt);
	return SQLITE_OK;
}

void PrintTask(Task ent)
{
	struct tm *timeinfo = gmtime(&ent.time);
	char datetime[40];
	int rc = strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M:%S", timeinfo);
	if (rc == 0)
	{
		ESP_LOGE(TAG, "Task::PrintEntry: strftime cannot process entry %lld!", ent.id);
	}

	ESP_LOGI(TAG, "ID: %lld | Name: %s | Due: %s (%ld) | Priority: %d | Completed: %d | Description: %s",
			 ent.id, ent.name, datetime, ent.time, ent.priority, ent.completion, ent.description);
}

int PrintTaskDB(sqlite3 *db, sqlite3_int64 id)
{
	Task ent;
	int rc = RetrieveTaskDB(db, id, &ent);
	if (rc != SQLITE_OK)
	{
		return rc;
	}
	PrintTask(ent);

	return SQLITE_OK;
}

int RemoveTaskDB(sqlite3 *db, sqlite3_int64 id)
{
	char *zErrMsg = 0;
	char sql[1024];
	snprintf(sql, sizeof(sql), "DELETE FROM tasks WHERE id = %lld;", id);

	int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		ESP_LOGE(TAG, "SQL::RemoveEntry: SQL error: %s", zErrMsg);
		sqlite3_free(zErrMsg);
		return rc;
	}

	return SQLITE_OK;
}

int RetrieveTasksSortedDB(sqlite3 *db, Task *taskMemory, int count)
{
	if (!taskMemory || !db)
	{
		ESP_LOGE(TAG, "RetrieveEntriesSorted: Received NULL pointer");
		return -1;
	}

	if (count < 1)
	{
		ESP_LOGW(TAG, "RetrieveEntriesSorted: Did you really want to retrieve 0 entries?");
		return 0; // Retrieve nothing
	}

	const char *sql = "SELECT id, name, datetime, priority, completed, description "
					  "FROM tasks "
					  "ORDER BY datetime ASC, priority DESC "
					  "LIMIT ?;";

	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		ESP_LOGE(TAG, "RetrieveEntriesSorted: Failed to prepare statement: %s", sqlite3_errmsg(db));
		return -2;
	}

	// Bind the limit parameter
	sqlite3_bind_int(stmt, 1, count);

	int i = 0;
	// Check while there is data to be read or if the requested entries have been obtained
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW && i < count)
	{
		Task *task = taskMemory + i;

		strncpy(task->id,sqlite3_column_int64(stmt, 0), UUID_SIZE);

		const unsigned char *nameText = sqlite3_column_text(stmt, 1);
		strncpy(task->name, nameText ? (const char *)nameText : "", MAX_TASK_NAME_SIZE - 1);
		task->name[MAX_TASK_NAME_SIZE - 1] = '\0';

		sqlite3_int64 timestamp = sqlite3_column_int64(stmt, 2);
		task->time = (time_t)timestamp;

		task->priority = (char)sqlite3_column_int(stmt, 3);
		task->completion = (CompletionStatus)sqlite3_column_int(stmt, 4);

		const unsigned char *descText = sqlite3_column_text(stmt, 5);
		strncpy(task->description, descText ? (const char *)descText : "", MAX_TASK_DESC_SIZE - 1);
		task->description[MAX_TASK_DESC_SIZE - 1] = '\0';

		i++;
	}

	if (rc != SQLITE_DONE)
	{
		ESP_LOGE(TAG, "RetrieveEntriesSorted: Error while reading rows: %s", sqlite3_errmsg(db));
		sqlite3_finalize(stmt);
		return -3;
	}

	sqlite3_finalize(stmt);
	return i; // Return number of rows actually retrieved
}

/// @brief Toggles the completion status of an entry in the tasks table
/// @param db Database
/// @param id The ID of the entry to be toggled
void CompleteTaskDB(sqlite3 *db, sqlite3_int64 id)
{
	char *zErrMsg = 0;
	char sql[1024];
	sqlite3_stmt *stmt;
	int rc;
	int current_status;

	// Prepare SQL statement to get the current completion status
	snprintf(sql, sizeof(sql), "SELECT completed FROM tasks WHERE id = %lld;", id);

	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
	if (rc != SQLITE_OK)
	{
		ESP_LOGE(TAG, "SQL::CompleteEntry: Failed to prepare statement: %s", sqlite3_errmsg(db));
		return;
	}

	// Execute the statement and get the current completion status
	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW)
	{
		current_status = sqlite3_column_int(stmt, 0);
	}
	else
	{
		ESP_LOGE(TAG, "SQL::CompleteEntry: Failed to get current status for ID %lld", id);
		sqlite3_finalize(stmt);
		return;
	}

	// Finalize the statement
	sqlite3_finalize(stmt);

	// Toggle the completion status
	int new_status = !current_status;

	// Prepare SQL statement to update the completion status
	snprintf(sql, sizeof(sql), "UPDATE tasks SET completed = %d WHERE id = %lld;", new_status, id);

	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg); // Run SQL command

	// Error checking
	if (rc != SQLITE_OK)
	{
		ESP_LOGE(TAG, "SQL::CompleteEntry: SQL error: %s", zErrMsg);
		sqlite3_free(zErrMsg);
		return;
	}

	ESP_LOGI(TAG, "SQL::CompleteEntry: Toggled completion status for ID %lld to %d", id, new_status);
}

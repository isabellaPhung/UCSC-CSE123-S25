#include "Logger.h"
#include "task.h"

#include <stdio.h>
#include <string.h>

int AddEntry(sqlite3 *db, sqlite3_int64 *id, const char *name, time_t datetime, char priority, CompletionStatus completed, const char *description)
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
		LOG_ERROR("Cannot prepare statement: %s", sqlite3_errmsg(db));
		return rc;
	}

	// Bind the values to the placeholders
	int index = 1;
	sqlite3_bind_text(stmt, index++, name, -1, SQLITE_STATIC);

	sqlite3_bind_int64(stmt, index++, (sqlite3_int64)datetime);
	sqlite3_bind_int(stmt, index++, (int)priority);
	sqlite3_bind_int(stmt, index++, (int)completed);

	sqlite3_bind_text(stmt, index++, description, -1, SQLITE_STATIC);

	// Execute the statement
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		LOG_ERROR("Execution failed: %s", sqlite3_errmsg(db));
		sqlite3_finalize(stmt);
		return rc;
	}

	// Clean up
	sqlite3_finalize(stmt);

	// Get the ID of the newly inserted row
	sqlite3_int64 ID = sqlite3_last_insert_rowid(db);
	*id = ID;

	LOG_INFO("SQL::AddEntry: Added %s with ID %lld", name, *id);

	return SQLITE_OK;
}

int RetrieveEntry(sqlite3 *db, sqlite3_int64 id, Task *ent)
{
	sqlite3_stmt *stmt;

	const char *sql = "SELECT * FROM tasks WHERE id = ?;";
	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
	sqlite3_bind_int64(stmt, 1, id);

	if (rc != SQLITE_OK)
	{
		LOG_ERROR("SQL::RetrieveEntry: Failed to fetch data: %s", sqlite3_errmsg(db));
		return rc;
	}

	//LOG_INFO("SQL::RetrieveEntry: Retrieving tasks at id: %d", id);

	// Execute the statement and fetch rows
	while (sqlite3_step(stmt) != SQLITE_DONE)
	{
		const char *name = (const char *)sqlite3_column_text(stmt, 1);
		time_t time = (time_t)sqlite3_column_int64(stmt, 2);
		char priority = (char)sqlite3_column_int(stmt, 3);
		CompletionStatus completed = (CompletionStatus)sqlite3_column_int(stmt, 4);
		const char *description = (const char *)sqlite3_column_text(stmt, 5);

		ent->id = id;

		if (strlen(name) > MAX_NAME)
		{
			LOG_WARNING("Task::RetrieveEntry: Task Name is oversized, string will be truncated!");
		}
		strncpy(ent->name, name, MAX_NAME);
		ent->time = time;
		ent->priority = priority;
		ent->completion = completed;
		if (description)
		{
			if (strlen(name) > MAX_NAME)
			{
				LOG_WARNING("Task::RetrieveEntry: Task Description is oversized, string will be truncated!");
			}
			strncpy(ent->description, description, MAX_DESC);
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
		LOG_ERROR("Task::PrintEntry: strftime cannot process entry %lld!", ent.id);
	}

	LOG_INFO("ID: %lld | Name: %s | Due: %s (%ld) | Priority: %d | Completed: %d | Description: %s",
			 ent.id, ent.name, datetime, ent.time, ent.priority, ent.completion, ent.description);
}

int PrintEntry(sqlite3 *db, sqlite3_int64 id)
{
	Task ent;
	int rc = RetrieveEntry(db, id, &ent);
	if (rc != SQLITE_OK)
	{
		return rc;
	}
	PrintTask(ent);

	return SQLITE_OK;
}

int RemoveEntry(sqlite3 *db, sqlite3_int64 id)
{
	char *zErrMsg = 0;
	char sql[1024];
	snprintf(sql, sizeof(sql), "DELETE FROM tasks WHERE id = %lld;", id);

	int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		LOG_ERROR("SQL::RemoveEntry: SQL error: %s", zErrMsg);
		sqlite3_free(zErrMsg);
		return rc;
	}

	return SQLITE_OK;
}

int RetrieveEntriesSorted(sqlite3 *db, Task *taskMemory, int count)
{
	if (!taskMemory || !db)
	{
		LOG_ERROR("RetrieveEntriesSorted: Received NULL pointer");
		return -1;
	}

	if (count < 1)
	{
		LOG_WARNING("RetrieveEntriesSorted: Did you really want to retrieve 0 entries?");
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
		LOG_ERROR("RetrieveEntriesSorted: Failed to prepare statement: %s", sqlite3_errmsg(db));
		return -2;
	}

	// Bind the limit parameter
	sqlite3_bind_int(stmt, 1, count);

	int i = 0;
	// Check while there is data to be read or if the requested entries have been obtained
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW && i < count)
	{
		Task *task = taskMemory + i;

		task->id = sqlite3_column_int64(stmt, 0);

		const unsigned char *nameText = sqlite3_column_text(stmt, 1);
		strncpy(task->name, nameText ? (const char *)nameText : "", MAX_NAME - 1);
		task->name[MAX_NAME - 1] = '\0';

		sqlite3_int64 timestamp = sqlite3_column_int64(stmt, 2);
		task->time = (time_t)timestamp;

		task->priority = (char)sqlite3_column_int(stmt, 3);
		task->completion = (CompletionStatus)sqlite3_column_int(stmt, 4);

		const unsigned char *descText = sqlite3_column_text(stmt, 5);
		strncpy(task->description, descText ? (const char *)descText : "", MAX_DESC - 1);
		task->description[MAX_DESC - 1] = '\0';

		i++;
	}

	if (rc != SQLITE_DONE)
	{
		LOG_ERROR("RetrieveEntriesSorted: Error while reading rows: %s", sqlite3_errmsg(db));
		sqlite3_finalize(stmt);
		return -3;
	}

	sqlite3_finalize(stmt);
	return i; // Return number of rows actually retrieved
}

/// @brief Toggles the completion status of an entry in the tasks table
/// @param db Database
/// @param id The ID of the entry to be toggled
void CompleteEntry(sqlite3 *db, sqlite3_int64 id)
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
		LOG_ERROR("SQL::CompleteEntry: Failed to prepare statement: %s", sqlite3_errmsg(db));
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
		LOG_ERROR("SQL::CompleteEntry: Failed to get current status for ID %lld", id);
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
		LOG_ERROR("SQL::CompleteEntry: SQL error: %s", zErrMsg);
		sqlite3_free(zErrMsg);
		return;
	}

	LOG_INFO("SQL::CompleteEntry: Toggled completion status for ID %lld to %d", id, new_status);
}

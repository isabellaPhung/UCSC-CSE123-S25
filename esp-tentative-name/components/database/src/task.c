#include "Logger.h"
#include "task.h"

#include <stdio.h>
#include <string.h>

/// @brief Adds an entry to the tasks table
/// @param db Database
/// @param id   Outputs the id of the entry
/// @param name Task name
/// @param datetim Datetime in Unix Timestamp format
/// @param priority The importance of the task (0-9)
/// @param completed    Whether the task is complete
/// @param description (optional) additional information about the task
/// @return SQLite Error
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

    LOG_INFO("Entry added!");
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

    LOG_INFO("SQL::RetrieveEntry: Retrieving tasks at id: %d", id);

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

/// @brief Prints data of a specified entry
/// @param db
/// @param id
int PrintEntry(sqlite3 *db, sqlite3_int64 id)
{
    Task ent;
    int rc = RetrieveEntry(db, id, &ent);
    if (rc != SQLITE_OK)
    {
        return rc;
    }

    struct tm *timeinfo = gmtime(&ent.time);
    char datetime[40];
    rc = strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M:%S", timeinfo);
    if (rc == 0)
    {
        LOG_ERROR("Task::PrintEntry: strftime cannot process entry %lld!", id);
    }

    printf("ID: %lld | Name: %s | Due: %s (%ld) | Priority: %d | Completed: %d | Description: %s\n",
           ent.id, ent.name, datetime, ent.time, ent.priority, ent.completion, ent.description);

    return SQLITE_OK;
}

/// @brief Deletes entry
/// @param db database to be altered
/// @param id entry id
/// @return SQLite Error
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
        LOG_ERROR("SQL::CompleteEntry: SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return;
    }

    LOG_INFO("SQL::CompleteEntry: Toggled completion status for ID %lld to %d", id, new_status);
}

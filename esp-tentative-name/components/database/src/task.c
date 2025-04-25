#include "Logger.h"
#include "task.h"

#include "../include/cJSON.h"
#include <stdio.h>
#include <string.h>

int AddTaskDB(sqlite3 *db, Task *ent)
{
    const char *sql = "INSERT INTO tasks (id, name, datetime, priority, completed, description) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        LOG_ERROR("Cannot prepare statement: %s", sqlite3_errmsg(db));
        return rc;
    }

    sqlite3_bind_text(stmt, 1, ent->uuid, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, ent->name, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, (sqlite3_int64)ent->time);
    sqlite3_bind_int(stmt, 4, (int)ent->priority);
    sqlite3_bind_int(stmt, 5, (int)ent->completion);
    sqlite3_bind_text(stmt, 6, ent->description, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        LOG_ERROR("Execution failed: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return rc;
    }

    sqlite3_finalize(stmt);
    LOG_INFO("SQL::AddEntry: Added <%s> with UUID <%s>", ent->name, ent->uuid);
    return SQLITE_OK;
}

// Adds JSON entries to database
int ParseTasksJSON(sqlite3 *db, const char *json)
{
    cJSON *root = cJSON_Parse(json);
    if (!root)
    {
        printf("Error parsing JSON\n");
        return -1;
    }

    cJSON *tasks = cJSON_GetObjectItem(root, "tasks");
    if (!cJSON_IsArray(tasks))
    {
        printf("Invalid JSON: 'tasks' should be an array\n");
        cJSON_Delete(root);
        return -2;
    }

    cJSON *item = NULL;
    cJSON_ArrayForEach(item, tasks)
    {
        Task task;
        memset(&task, 0, sizeof(Task)); // Clear struct

        const cJSON *id = cJSON_GetObjectItem(item, "id");
        const cJSON *name = cJSON_GetObjectItem(item, "name");
        const cJSON *description = cJSON_GetObjectItem(item, "description");
        const cJSON *completion = cJSON_GetObjectItem(item, "completion");
        const cJSON *priority = cJSON_GetObjectItem(item, "priority");
        const cJSON *duedate = cJSON_GetObjectItem(item, "duedate");

        if (!cJSON_IsString(id) || !cJSON_IsString(name) || !cJSON_IsNumber(priority) || !cJSON_IsNumber(duedate))
        {
            LOG_ERROR("Missing or invalid task fields\n");
            continue;
        }

        strncpy(task.uuid, id->valuestring, UUID_LENGTH - 1);
        strncpy(task.name, name->valuestring, MAX_TASK_NAME_SIZE - 1);
        task.time = (time_t)duedate->valuedouble;
        task.priority = priority->valuestring[0];

        if (cJSON_IsString(completion) && strcmp(completion->valuestring, "complete") == 0)
        {
            task.completion = COMPLETE;
        }
        else
        {
            task.completion = INCOMPLETE;
        }

        if (cJSON_IsString(description))
        {
            strncpy(task.description, description->valuestring, MAX_TASK_DESC_SIZE - 1);
        }

        int rc = AddTaskDB(db, &task);
        if (rc != SQLITE_OK)
        {
            printf("Failed to insert task %s: %s\n", task.uuid, sqlite3_errmsg(db));
        }
    }

    cJSON_Delete(root);
    return 0;
}

int RetrieveTaskDB(sqlite3 *db, const char *uuid, Task *ent)
{
    const char *sql = "SELECT * FROM tasks WHERE id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, uuid, -1, SQLITE_STATIC);

    if (rc != SQLITE_OK)
    {
        LOG_ERROR("SQL::RetrieveTaskDB: Failed to fetch data: %s", sqlite3_errmsg(db));
        return rc;
    }

    while (sqlite3_step(stmt) != SQLITE_DONE)
    {
        strncpy(ent->uuid, uuid, UUID_LENGTH);
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        time_t time = (time_t)sqlite3_column_int64(stmt, 2);
        char priority = (char)sqlite3_column_int(stmt, 3);
        CompletionStatus completed = (CompletionStatus)sqlite3_column_int(stmt, 4);
        const char *description = (const char *)sqlite3_column_text(stmt, 5);

        strncpy(ent->name, name, MAX_TASK_NAME_SIZE);
        ent->time = time;
        ent->priority = priority;
        ent->completion = completed;
        strncpy(ent->description, description ? description : "", MAX_TASK_DESC_SIZE);
    }

    sqlite3_finalize(stmt);
    return SQLITE_OK;
}

void PrintTask(Task ent)
{
    struct tm *timeinfo = gmtime(&ent.time);
    char datetime[40];
    strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M:%S", timeinfo);
    LOG_INFO("UUID: %s | Name: %s | Due: %s (%ld) | Priority: %d | Completed: %d | Description: %s",
             ent.uuid, ent.name, datetime, ent.time, ent.priority, ent.completion, ent.description);
}

int PrintTaskDB(sqlite3 *db, const char *uuid)
{
    Task ent;
    int rc = RetrieveTaskDB(db, uuid, &ent);
    if (rc != SQLITE_OK)
        return rc;
    PrintTask(ent);
    return SQLITE_OK;
}

int RemoveTaskDB(sqlite3 *db, const char *uuid)
{
    char sql[256];
    snprintf(sql, sizeof(sql), "DELETE FROM tasks WHERE id = '%s';", uuid);
    char *zErrMsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        LOG_ERROR("SQL::RemoveEntry: SQL error: %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return rc;
    }
    return SQLITE_OK;
}

int RetrieveTasksSortedDB(sqlite3 *db, Task *taskMemory, int count)
{
    if (!taskMemory || !db || count < 1)
        return -1;

    const char *sql = "SELECT id, name, datetime, priority, completed, description FROM tasks ORDER BY datetime ASC, priority DESC LIMIT ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return -2;

    sqlite3_bind_int(stmt, 1, count);
    int i = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW && i < count)
    {
        Task *task = taskMemory + i;
        strncpy(task->uuid, (const char *)sqlite3_column_text(stmt, 0), UUID_LENGTH);
        strncpy(task->name, (const char *)sqlite3_column_text(stmt, 1), MAX_TASK_NAME_SIZE);
        task->time = (time_t)sqlite3_column_int64(stmt, 2);
        task->priority = (char)sqlite3_column_int(stmt, 3);
        task->completion = (CompletionStatus)sqlite3_column_int(stmt, 4);
        strncpy(task->description, (const char *)sqlite3_column_text(stmt, 5), MAX_TASK_DESC_SIZE);
        i++;
    }

    sqlite3_finalize(stmt);
    return i;
}

void CompleteTaskDB(sqlite3 *db, const char *uuid)
{
    char sql[256];
    sqlite3_stmt *stmt;
    int rc;
    int current_status;

    snprintf(sql, sizeof(sql), "SELECT completed FROM tasks WHERE id = '%s';", uuid);
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return;
    if ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        current_status = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    int new_status = !current_status;
    snprintf(sql, sizeof(sql), "UPDATE tasks SET completed = %d WHERE id = '%s';", new_status, uuid);
    char *zErrMsg = NULL;
    rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        LOG_ERROR("SQL::CompleteEntry: SQL error: %s", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    LOG_INFO("SQL::CompleteEntry: Toggled completion status for UUID %s to %d", uuid, new_status);
}

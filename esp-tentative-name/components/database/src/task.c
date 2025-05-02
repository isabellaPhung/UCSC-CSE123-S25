#include "task.h"

#include "cJSON.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>

int AddTaskDB(sqlite3 *db, task_t *ent)
{
    static const char *TAG = "task::AddTaskDB";

    const char *sql = "INSERT INTO tasks (id, name, datetime, priority, completed, description) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Cannot prepare statement: %s", sqlite3_errmsg(db));
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
        ESP_LOGE(TAG, "Execution failed: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return rc;
    }

    sqlite3_finalize(stmt);
    ESP_LOGI(TAG, "Added <%s> with UUID <%s>", ent->name, ent->uuid);
    return SQLITE_OK;
}

// Adds JSON entries to database
int ParseTasksJSON(sqlite3 *db, const char *json)
{
    static const char *TAG = "task::ParseTasksJSON";

    cJSON *root = cJSON_Parse(json);
    if (!root)
    {
        ESP_LOGE(TAG, "Error parsing JSON");
        return -1;
    }

    cJSON *taskItem = cJSON_GetObjectItem(root, "task");
    if (!cJSON_IsObject(taskItem))
    {
        ESP_LOGE(TAG, "Invalid JSON: 'task' should be a JSON object");
        cJSON_Delete(root);
        return -2;
    }

    task_t task = {0};

    const cJSON *id = cJSON_GetObjectItem(taskItem, "id");
    const cJSON *name = cJSON_GetObjectItem(taskItem, "name");
    const cJSON *description = cJSON_GetObjectItem(taskItem, "description");
    const cJSON *completion = cJSON_GetObjectItem(taskItem, "completion"); // TODO: Convert to integer input
    const cJSON *priority = cJSON_GetObjectItem(taskItem, "priority");
    const cJSON *duedate = cJSON_GetObjectItem(taskItem, "duedate");

    if (!cJSON_IsString(id) || !cJSON_IsString(name) || !cJSON_IsString(priority) || !cJSON_IsNumber(duedate))
    {
        ESP_LOGE(TAG, "Missing or invalid task fields");
        return -3;
    }

    strncpy(task.uuid, id->valuestring, UUID_LENGTH - 1);
    strncpy(task.name, name->valuestring, MAX_TASK_NAME_SIZE - 1);
    task.time = (time_t)duedate->valueint;

    // Parse priority enum
    char priorityVal;
    if (!strcmp(priority->valuestring, "high"))
    {
        priorityVal = 3;
    }
    else if (!strcmp(priority->valuestring, "medium"))
    {
        priorityVal = 2;
    }
    else
    {
        priorityVal = 1;
    }

    task.priority = priorityVal;

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
        ESP_LOGI(TAG, "Failed to insert task %s: %s", task.uuid, sqlite3_errmsg(db));
    }

    cJSON_Delete(root);
    return 0;
}

int RetrieveTaskDB(sqlite3 *db, const char *uuid, task_t *ent)
{
    static const char *TAG = "task::RetrieveTaskDB";

    const char *sql = "SELECT * FROM tasks WHERE id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, uuid, -1, SQLITE_STATIC);

    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to fetch data: %s", sqlite3_errmsg(db));
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

void PrintTask(task_t ent)
{
    static const char *TAG = "task::PrintTask";

    struct tm *timeinfo = gmtime(&ent.time);
    char datetime[40];
    strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M:%S", timeinfo);
    ESP_LOGI(TAG, "UUID: %s | Name: %s | Due: %s (%lld) | Priority: %d | Completed: %d | Description: %s",
             ent.uuid, ent.name, datetime, ent.time, ent.priority, ent.completion, ent.description);
}

int PrintTaskDB(sqlite3 *db, const char *uuid)
{
    static const char *TAG = "task::PrintTaskDB";

    task_t ent;
    int rc = RetrieveTaskDB(db, uuid, &ent);
    if (rc != SQLITE_OK)
        return rc;
    PrintTask(ent);
    return SQLITE_OK;
}

int RemoveTaskDB(sqlite3 *db, const char *uuid)
{
    static const char *TAG = "task::RemoveTaskDB";

    char sql[256];
    snprintf(sql, sizeof(sql), "DELETE FROM tasks WHERE id = '%s';", uuid);
    char *zErrMsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "SQL error: %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return rc;
    }
    return SQLITE_OK;
}

int RetrieveTasksSortedDB(sqlite3 *db, task_t *taskMemory, int count)
{
    static const char *TAG = "task::RetrieveTasksSortedDB";

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
        task_t *task = taskMemory + i;
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
    static const char *TAG = "task::CompleteTaskDB";

    char sql[256];
    sqlite3_stmt *stmt;
    int rc;
    int current_status = 0;

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
        ESP_LOGE(TAG, "SQL error: %s", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    ESP_LOGI(TAG, "Toggled completion status for UUID %s to %d", uuid, new_status);
}

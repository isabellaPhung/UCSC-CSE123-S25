#include "task.h"

#include "cJSON.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>

esp_err_t AddTaskDB(sqlite3 *db, task_t *ent);
esp_err_t RemoveTaskDB(sqlite3 *db, const char *uuid);

// Adds JSON entries to database
esp_err_t ParseTasksJSON(sqlite3 *db, const char *json)
{
    static const char *TAG = "task::ParseTasksJSON";

    cJSON *root = cJSON_Parse(json);
    if (!root)
    {
        ESP_LOGE(TAG, "Error parsing JSON");
        return ESP_ERR_INVALID_ARG;
    }

    cJSON *taskItem = cJSON_GetObjectItem(root, "task");
    if (!cJSON_IsObject(taskItem))
    {
        ESP_LOGE(TAG, "Invalid JSON: 'task' should be a JSON object");
        cJSON_Delete(root);
        return ESP_ERR_INVALID_ARG;
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
        return ESP_ERR_INVALID_ARG;
    }

    strncpy(task.uuid, id->valuestring, UUID_LENGTH - 1);
    strncpy(task.name, name->valuestring, MAX_NAME_SIZE - 1);
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
        strncpy(task.description, description->valuestring, MAX_DESC_SIZE - 1);
    }

    int rc = AddTaskDB(db, &task);
    cJSON_Delete(root);

    if (rc != SQLITE_OK)
    {
        ESP_LOGI(TAG, "Failed to insert task %s: %s", task.uuid, sqlite3_errmsg(db));
        return ESP_FAIL;
    }

    return 0;
}

int RetrieveTasksSortedDB(sqlite3 *db, task_t *taskBuffer, int count, int offset)
{
    static const char *TAG = "task::RetrieveTasksSortedDB";

    if (!taskBuffer || count <= 0)
    {
        ESP_LOGE(TAG, "Invalid buffer or count");
        return -1;
    }

    // Currently sorts by just TIME
    const char *sql = "SELECT id, name, datetime, priority, completed, description "
                      "FROM tasks "
                      "ORDER BY (datetime - (priority * " PRIORITY_MULTIPLIER ")) ASC "
                      "LIMIT ? OFFSET ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to prepare SELECT statement: %s", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, count);
    sqlite3_bind_int(stmt, 2, offset);

    int i = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW && i < count)
    {
        const unsigned char *id = sqlite3_column_text(stmt, 0);
        const unsigned char *name = sqlite3_column_text(stmt, 1);
        sqlite3_int64 datetime = sqlite3_column_int64(stmt, 2);
        int priority = sqlite3_column_int(stmt, 3);
        int completed = sqlite3_column_int(stmt, 4);
        const unsigned char *desc = sqlite3_column_text(stmt, 5);

        strncpy(taskBuffer[i].uuid, (const char *)id, UUID_LENGTH - 1);
        taskBuffer[i].uuid[UUID_LENGTH - 1] = '\0';

        strncpy(taskBuffer[i].name, (const char *)name, MAX_NAME_SIZE - 1);
        taskBuffer[i].name[MAX_NAME_SIZE - 1] = '\0';

        taskBuffer[i].time = (time_t)datetime;
        taskBuffer[i].priority = (char)priority;
        taskBuffer[i].completion = (completed == 1) ? COMPLETE : INCOMPLETE;

        if (desc)
        {
            strncpy(taskBuffer[i].description, (const char *)desc, MAX_DESC_SIZE - 1);
            taskBuffer[i].description[MAX_DESC_SIZE - 1] = '\0';
        }
        else
        {
            taskBuffer[i].description[0] = '\0';
        }

        i++;
    }

    if (rc != SQLITE_DONE)
    {
        ESP_LOGE(TAG, "Error retrieving tasks: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    ESP_LOGI(TAG, "Retrieved %d task(s) starting at offset %d", i, offset);
    return i;
}

// --------------------------------------- Modify tasks -------------------------------------------

esp_err_t UpdateTaskStatusDB(sqlite3 *db, const char *uuid, TASK_STATUS new_status)
{
    static const char *TAG = "task::UpdateTaskStatusDB";

    if (new_status == MFD)
    {
        return RemoveTaskDB(db, uuid);
    }

    const char *sql = "UPDATE tasks SET completed = ? WHERE id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to prepare UPDATE statement: %s", sqlite3_errmsg(db));
        return ESP_FAIL;
    }

    sqlite3_bind_int(stmt, 1, (int)new_status);
    sqlite3_bind_text(stmt, 2, uuid, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        ESP_LOGE(TAG, "Failed to execute UPDATE statement: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return ESP_FAIL;
    }

    sqlite3_finalize(stmt);
    ESP_LOGI(TAG, "Updated task ID %s to status %d", uuid, new_status);
    return ESP_OK;
}

esp_err_t RemoveTaskDB(sqlite3 *db, const char *uuid)
{
    static const char *TAG = "task::CompleteTaskDB";

    const char *sql = "DELETE FROM tasks WHERE id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to prepare DELETE statement: %s", sqlite3_errmsg(db));
        return ESP_FAIL;
    }

    sqlite3_bind_text(stmt, 1, uuid, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        ESP_LOGE(TAG, "Failed to execute DELETE statement: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return ESP_FAIL;
    }

    sqlite3_finalize(stmt);
    ESP_LOGI(TAG, "Deleted task with ID: %s", uuid);
    return ESP_OK;
}
// -------------------------------------- Helper Functions ----------------------------------------

esp_err_t AddTaskDB(sqlite3 *db, task_t *ent)
{
    static const char *TAG = "task::AddTaskDB";

    const char *sql = "INSERT INTO tasks (id, name, datetime, priority, completed, description) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Cannot prepare statement: %s", sqlite3_errmsg(db));
        return ESP_FAIL;
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
        return ESP_FAIL;
    }

    sqlite3_finalize(stmt);
    ESP_LOGI(TAG, "Added <%s> with UUID <%s>", ent->name, ent->uuid);
    return ESP_OK;
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

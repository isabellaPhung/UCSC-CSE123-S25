#include "database.h"
#include "task.h"

#include "cJSON.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>

esp_err_t AddTaskDB(task_t *ent);
esp_err_t RemoveTaskDB(const char *uuid);

// Adds JSON entries to database
esp_err_t ParseTasksJSON(cJSON *taskItem)
{
    static const char *TAG = "task::ParseTasksJSON";
    heap_caps_monitor_local_minimum_free_size_start();

    if (!cJSON_IsObject(taskItem))
    {
        ESP_LOGE(TAG, "Invalid JSON: 'task' should be a JSON object");
        return ESP_ERR_INVALID_ARG;
    }

    // Allocate task on heap
    task_t *task = (task_t *)calloc(1, sizeof(task_t));
    ESP_LOGW(TAG, "Min heap after task alloc: %d bytes",
             heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT));
    if (!task)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for task");
        ESP_LOGW(TAG, "Free heap: %lu bytes", esp_get_free_heap_size());
        return ESP_ERR_NO_MEM;
    }

    // Extract JSON fields
    const cJSON *id = cJSON_GetObjectItem(taskItem, "id");
    const cJSON *name = cJSON_GetObjectItem(taskItem, "name");
    const cJSON *description = cJSON_GetObjectItem(taskItem, "description");
    const cJSON *completion = cJSON_GetObjectItem(taskItem, "completion");
    const cJSON *priority = cJSON_GetObjectItem(taskItem, "priority");
    const cJSON *duedate = cJSON_GetObjectItem(taskItem, "duedate");

    ESP_LOGW(TAG, "Min heap after cJSON extraction: %d bytes",
             heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT));

    // Validate required fields
    if (!cJSON_IsString(id) || !cJSON_IsString(name) ||
        !cJSON_IsNumber(priority) || !cJSON_IsNumber(duedate) ||
        !cJSON_IsNumber(completion))
    {
        ESP_LOGE(TAG, "Missing or invalid task fields");
        free(task);
        return ESP_ERR_INVALID_ARG;
    }

    if ((TASK_STATUS)completion->valueint == MFD)
    {
        ESP_LOGW(TAG, "Entry is marked for deletion, ignoring");
        free(task);
        return ESP_ERR_INVALID_ARG;
    }

    // Check lengths
    if (strlen(id->valuestring) >= UUID_LENGTH)
    {
        ESP_LOGE(TAG, "UUID too long (max %d)", UUID_LENGTH - 1);
        free(task);
        return ESP_ERR_INVALID_ARG;
    }

    if (strlen(name->valuestring) >= MAX_NAME_SIZE)
    {
        ESP_LOGW(TAG, "Name too long (max %d), truncating", MAX_NAME_SIZE - 1);
    }

    strncpy(task->uuid, id->valuestring, UUID_LENGTH - 1);
    strncpy(task->name, name->valuestring, MAX_NAME_SIZE - 1);
    task->time = (time_t)duedate->valueint;
    task->priority = (char)priority->valueint;
    task->completion = (TASK_STATUS)completion->valueint;

    if (cJSON_IsString(description) && strlen(description->valuestring) < MAX_DESC_SIZE)
    {
        strncpy(task->description, description->valuestring, MAX_DESC_SIZE - 1);
    }

    int rc = AddTaskDB(task);
    free(task);

    return rc;
}

int RetrieveTasksSortedDB(task_t *taskBuffer, int count, int offset)
{
    static const char *TAG = "task::RetrieveTasksSortedDB";

    if (!taskBuffer || count <= 0)
    {
        ESP_LOGE(TAG, "Invalid buffer or count");
        return -1;
    }

    sqlite3 *db = get_db_connection();

    // Currently sorts by just TIME
    static const char *sql = "SELECT id, name, datetime, priority, completed, description "
                             "FROM tasks "
                             "ORDER BY (datetime - (priority * " PRIORITY_MULTIPLIER ")) ASC "
                             "LIMIT ? OFFSET ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to prepare SELECT statement: %s", sqlite3_errmsg(db));
        release_db_connection();
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
        release_db_connection();
        return -1;
    }

    sqlite3_finalize(stmt);
    release_db_connection();
    ESP_LOGI(TAG, "Retrieved %d task(s) starting at offset %d", i, offset);
    return i;
}

// --------------------------------------- Modify tasks -------------------------------------------

esp_err_t UpdateTaskStatusDB(const char *uuid, TASK_STATUS new_status)
{
    static const char *TAG = "task::UpdateTaskStatusDB";

    if (new_status == MFD)
    {
        return RemoveTaskDB(uuid);
    }

    sqlite3 *db = get_db_connection();

    static const char *sql = "UPDATE tasks SET completed = ? WHERE id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to prepare UPDATE statement: %s", sqlite3_errmsg(db));
        release_db_connection();
        return ESP_FAIL;
    }

    sqlite3_bind_int(stmt, 1, (int)new_status);
    sqlite3_bind_text(stmt, 2, uuid, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        ESP_LOGE(TAG, "Failed to execute UPDATE statement: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        release_db_connection();
        return ESP_FAIL;
    }

    // Clean up
    sqlite3_finalize(stmt);
    release_db_connection();

    ESP_LOGI(TAG, "Updated task ID %s to status %d", uuid, new_status);
    return ESP_OK;
}

esp_err_t RemoveTaskDB(const char *uuid)
{
    static const char *TAG = "task::CompleteTaskDB";

    sqlite3 *db = get_db_connection();

    static const char *sql = "DELETE FROM tasks WHERE id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to prepare DELETE statement: %s", sqlite3_errmsg(db));
        release_db_connection();
        return ESP_FAIL;
    }

    sqlite3_bind_text(stmt, 1, uuid, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        ESP_LOGE(TAG, "Failed to execute DELETE statement: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        release_db_connection();
        return ESP_FAIL;
    }

    // Clean up
    sqlite3_finalize(stmt);
    release_db_connection();

    ESP_LOGI(TAG, "Deleted task with ID: %s", uuid);
    return ESP_OK;
}
// -------------------------------------- Helper Functions ----------------------------------------

esp_err_t AddTaskDB(task_t *ent)
{
    static const char *TAG = "task::AddTaskDB";
    ESP_LOGW(TAG, "Free heap: %lu bytes", esp_get_free_heap_size());

    sqlite3 *db = get_db_connection();

    // Use one efficient SQL statement instead of two separate ones
    const char *upsert_sql =
        "INSERT OR REPLACE INTO tasks (id, name, datetime, priority, completed, description) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, upsert_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Cannot prepare upsert statement: %s", sqlite3_errmsg(db));
        release_db_connection();
        return ESP_FAIL;
    }

    // Bind parameters using SQLITE_TRANSIENT to avoid memory issues
    sqlite3_bind_text(stmt, 1, ent->uuid, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, ent->name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 3, (sqlite3_int64)ent->time);
    sqlite3_bind_int(stmt, 4, (int)ent->priority);
    sqlite3_bind_int(stmt, 5, (int)ent->completion);
    sqlite3_bind_text(stmt, 6, ent->description, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    release_db_connection();

    if (rc != SQLITE_DONE)
    {
        ESP_LOGE(TAG, "Upsert failed: %s", sqlite3_errmsg(db));
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Upserted task with UUID <%s>", ent->uuid);
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

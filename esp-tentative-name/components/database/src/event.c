#include "event.h"
#include <string.h>

#include "esp_system.h"
#include "esp_log.h"

esp_err_t AddEventDB(sqlite3 *db, const event_t *event);

int RetrieveEventsSortedDB(sqlite3 *db, event_t *eventBuffer, int count, int offset)
{
    const char *TAG = "event::RetrieveEventsSortedDB";

    if (!eventBuffer || count <= 0)
    {
        ESP_LOGE(TAG, "Invalid buffer or count");
        return -1;
    }

    const char *sql = "SELECT id, name, starttime, duration, description "
                      "FROM events "
                      "ORDER BY starttime ASC "
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
        sqlite3_int64 starttime = sqlite3_column_int64(stmt, 2);
        sqlite3_int64 duration = sqlite3_column_int64(stmt, 3);
        const unsigned char *desc = sqlite3_column_text(stmt, 4);

        strncpy(eventBuffer[i].uuid, (const char *)id, UUID_LENGTH - 1);
        eventBuffer[i].uuid[UUID_LENGTH - 1] = '\0';
        strncpy(eventBuffer[i].name, (const char *)name, MAX_NAME_SIZE - 1);
        eventBuffer[i].name[MAX_NAME_SIZE - 1] = '\0';
        eventBuffer[i].start_time = (time_t)starttime;
        eventBuffer[i].duration = (time_t)duration;

        if (desc)
        {
            strncpy(eventBuffer[i].description, (const char *)desc, MAX_DESC_SIZE - 1);
            eventBuffer[i].description[MAX_DESC_SIZE - 1] = '\0';
        }
        else
        {
            eventBuffer[i].description[0] = '\0';
        }

        i++;
    }

    if (rc != SQLITE_DONE)
    {
        ESP_LOGE(TAG, "Error retrieving events: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    ESP_LOGI(TAG, "Retrieved %d event(s) starting at offset %d", i, offset);
    return i;
}

esp_err_t ParseEventsJSON(sqlite3 *db, const cJSON *eventItem)
{
    const char *TAG = "event::ParseEventsJSON";

    if (!db)
    {
        ESP_LOGE(TAG, "Invalid input: Missing database");
        return ESP_ERR_INVALID_ARG;
    }

    if (!cJSON_IsObject(eventItem))
    {
        ESP_LOGE(TAG, "Invalid JSON: 'event' should be a JSON object");
        return ESP_ERR_INVALID_ARG;
    }

    event_t *event = (event_t *)calloc(1, sizeof(event_t));
    if (!event)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for event");
        ESP_LOGW(TAG, "Free heap: %lu bytes", esp_get_free_heap_size());
        return ESP_ERR_NO_MEM;
    }

    // UUID
    cJSON *id = cJSON_GetObjectItem(eventItem, "id");
    if (!cJSON_IsString(id) || strlen(id->valuestring) >= UUID_LENGTH)
    {
        ESP_LOGE(TAG, "Invalid or missing 'id'");
        free(event);
        return ESP_ERR_INVALID_ARG;
    }
    strncpy(event->uuid, id->valuestring, UUID_LENGTH - 1);

    // Name
    cJSON *name = cJSON_GetObjectItem(eventItem, "name");
    if (!cJSON_IsString(name) || strlen(name->valuestring) >= MAX_NAME_SIZE)
    {
        ESP_LOGE(TAG, "Invalid or missing 'name'");
        free(event);
        return ESP_ERR_INVALID_ARG;
    }
    strncpy(event->name, name->valuestring, MAX_NAME_SIZE - 1);

    // Description
    cJSON *desc = cJSON_GetObjectItem(eventItem, "description");
    if (!cJSON_IsString(desc) || strlen(desc->valuestring) >= MAX_DESC_SIZE)
    {
        ESP_LOGE(TAG, "Invalid or missing 'description'");
        free(event);
        return ESP_ERR_INVALID_ARG;
    }
    strncpy(event->description, desc->valuestring, MAX_DESC_SIZE - 1);

    // Start Time
    cJSON *starttime = cJSON_GetObjectItem(eventItem, "starttime");
    if (!cJSON_IsNumber(starttime))
    {
        ESP_LOGE(TAG, "Invalid or missing 'starttime'");
        free(event);
        return ESP_ERR_INVALID_ARG;
    }
    event->start_time = (time_t)starttime->valuedouble;

    // Duration
    cJSON *duration = cJSON_GetObjectItem(eventItem, "duration");
    if (!cJSON_IsNumber(duration))
    {
        ESP_LOGE(TAG, "Invalid or missing 'duration'");
        free(event);
        return ESP_ERR_INVALID_ARG;
    }
    event->duration = (time_t)duration->valuedouble;

    // Call AddEventDB
    esp_err_t result = AddEventDB(db, event);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add event to DB");
    }

    return result;
}

// -------------------------------------- Helper Scripts ------------------------------------------

esp_err_t AddEventDB(sqlite3 *db, const event_t *event)
{
    const char *TAG = "event::AddEventDB";
    ESP_LOGI(TAG, "Free heap: %lu bytes", esp_get_free_heap_size());

    // Try to UPDATE first
    const char *update_sql =
        "UPDATE events SET name = ?, starttime = ?, duration = ?, description = ? WHERE id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, update_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Prepare update failed: %s", sqlite3_errmsg(db));
        return ESP_FAIL;
    }

    sqlite3_bind_text(stmt, 1, event->name, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, event->start_time);
    sqlite3_bind_int64(stmt, 3, event->duration);
    sqlite3_bind_text(stmt, 4, event->description, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, event->uuid, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE)
    {
        ESP_LOGE(TAG, "Update failed: %s", sqlite3_errmsg(db));
        return ESP_FAIL;
    }

    if (sqlite3_changes(db) > 0)
    {
        ESP_LOGI(TAG, "Updated event with ID: %s", event->uuid);
        return ESP_OK;
    }

    // Insert if no update occurred
    const char *insert_sql =
        "INSERT INTO events (id, name, starttime, duration, description) VALUES (?, ?, ?, ?, ?);";
    rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Prepare insert failed: %s", sqlite3_errmsg(db));
        return ESP_FAIL;
    }

    sqlite3_bind_text(stmt, 1, event->uuid, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, event->name, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, event->start_time);
    sqlite3_bind_int64(stmt, 4, event->duration);
    sqlite3_bind_text(stmt, 5, event->description, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE)
    {
        ESP_LOGE(TAG, "Insert failed: %s", sqlite3_errmsg(db));
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Inserted new event with ID: %s", event->uuid);
    return ESP_OK;
}

esp_err_t RemoveEventDB(sqlite3 *db, const char *uuid)
{
    const char *TAG = "event::RemoveEventDB";

    const char *sql = "DELETE FROM events WHERE id = ?;";
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

    ESP_LOGI(TAG, "Deleted event with ID: %s", uuid);
    sqlite3_finalize(stmt);
    return ESP_OK;
}

// ----------------------------------------- TEST SCRIPT ------------------------------------------
/*
esp_err_t TestEventFunctions(sqlite3 *db)
{
    const char *TAG = "event::TestEventFunctions";

    if (!db)
    {
        ESP_LOGE(TAG, "Invalid database handle");
        return ESP_ERR_INVALID_ARG;
    }

    // Define JSON strings with unique UUIDs
    const char *json_events[3] = {
        "{\"event\": {\"id\": \"uuid-001\", \"name\": \"Event One\", \"description\": \"First Event\", \"starttime\": 1762266000, \"duration\": 3600}}",
        "{\"event\": {\"id\": \"uuid-002\", \"name\": \"Event Two\", \"description\": \"Second Event\", \"starttime\": 1762270000, \"duration\": 1800}}",
        "{\"event\": {\"id\": \"uuid-003\", \"name\": \"Event Three\", \"description\": \"Third Event\", \"starttime\": 1762280000, \"duration\": 7200}}"};

    // Insert each JSON event
    for (int i = 0; i < 3; i++)
    {
        ESP_LOGI(TAG, "Parsing and inserting event %d", i + 1);
        esp_err_t err = ParseEventsJSON(db, json_events[i]);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to insert event %d", i + 1);
            return err;
        }
    }

    // Retrieve events sorted
    event_t buffer[3] = {0};
    ESP_LOGI(TAG, "Retrieving sorted events...");
    int res = RetrieveEventsSortedDB(db, buffer, 3, 0);
    if (res < 3)
    {
        ESP_LOGE(TAG, "Failed to retrieve sorted events");
        return ESP_FAIL;
    }

    // Log events
    for (int i = 0; i < 3; i++)
    {
        ESP_LOGI(TAG, "Event %d:", i + 1);
        ESP_LOGI(TAG, " UUID: %s", buffer[i].uuid);
        ESP_LOGI(TAG, " Name: %s", buffer[i].name);
        ESP_LOGI(TAG, " Description: %s", buffer[i].description);
        ESP_LOGI(TAG, " Start Time: %lld", buffer[i].start_time);
        ESP_LOGI(TAG, " Duration: %lld", buffer[i].duration);
    }

    // Remove all inserted events
    for (int i = 0; i < 3; i++)
    {
        cJSON *root = cJSON_Parse(json_events[i]);
        if (!root)
        {
            ESP_LOGE(TAG, "Failed to parse JSON again for cleanup");
            return ESP_FAIL;
        }
        cJSON *event = cJSON_GetObjectItem(root, "event");
        cJSON *id = cJSON_GetObjectItem(event, "id");
        if (!cJSON_IsString(id))
        {
            cJSON_Delete(root);
            ESP_LOGE(TAG, "Failed to extract UUID during cleanup");
            return ESP_FAIL;
        }

        ESP_LOGI(TAG, "Removing event with UUID: %s", id->valuestring);
        esp_err_t err = RemoveEventDB(db, id->valuestring);
        cJSON_Delete(root);

        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to remove event %d", i + 1);
            return err;
        }
    }

    ESP_LOGI(TAG, "All event tests passed and cleaned up.");
    return ESP_OK;
}
*/

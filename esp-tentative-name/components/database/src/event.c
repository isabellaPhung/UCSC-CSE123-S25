#include "event.h"
#include <string.h>

#include "esp_log.h"

esp_err_t RetrieveEventsSortedDB(sqlite3 *db, event_t *eventBuffer, int count, int offset)
{
    static const char *TAG = "event::RemoveEventDB";

    if (!eventBuffer || count <= 0)
    {
        ESP_LOGE(TAG, "Invalid buffer or count");
        return ESP_ERR_INVALID_ARG;
    }

    const char *sql = "SELECT id, name, starttime, duration, description "
                      "FROM events ORDER BY starttime ASC LIMIT ? OFFSET ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to prepare SELECT statement: %s", sqlite3_errmsg(db));
        return ESP_FAIL;
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
        return ESP_FAIL;
    }

    sqlite3_finalize(stmt);
    ESP_LOGI(TAG, "Retrieved %d event(s) starting at offset %d", i, offset);
    return ESP_OK;
}

// -------------------------------------- Helper Scripts ------------------------------------------

esp_err_t AddEventDB(sqlite3 *db, const event_t *event)
{
    static const char *TAG = "event::AddEventDB";

    const char *sql = "INSERT INTO events (id, name, starttime, duration, description) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to prepare INSERT statement: %s", sqlite3_errmsg(db));
        return ESP_FAIL;
    }

    sqlite3_bind_text(stmt, 1, event->uuid, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, event->name, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, event->start_time);
    sqlite3_bind_int64(stmt, 4, event->duration);
    sqlite3_bind_text(stmt, 5, event->description, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        ESP_LOGE(TAG, "Failed to execute INSERT statement: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Inserted event with ID: %s", event->uuid);
    sqlite3_finalize(stmt);
    return ESP_OK;
}

esp_err_t RemoveEventDB(sqlite3 *db, const char *uuid)
{
    static const char *TAG = "event::RemoveEventDB";

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

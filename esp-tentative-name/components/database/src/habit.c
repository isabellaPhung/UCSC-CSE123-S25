#include "habit.h"
#include "esp_log.h"
#include "esp_system.h"

#include <cJSON.h>
#include <string.h>

int RetrieveHabitsDB(sqlite3 *db, habit_t *habitBuffer, int count, int offset)
{
    const char *TAG = "habit::RetrieveHabitsDB";

    if (!db || !habitBuffer || count <= 0 || offset < 0)
    {
        ESP_LOGE(TAG, "Invalid arguments");
        return -1;
    }

    const char *sql = "SELECT id, name FROM habits LIMIT ? OFFSET ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to prepare statement: %s", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, count);
    sqlite3_bind_int(stmt, 2, offset);

    int idx = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW && idx < count)
    {
        const unsigned char *id = sqlite3_column_text(stmt, 0);
        const unsigned char *name = sqlite3_column_text(stmt, 1);
        int goal = sqlite3_column_int(stmt, 2);

        if (!id || !name)
        {
            ESP_LOGW(TAG, "Null field detected at row %d", idx);
            continue;
        }

        strncpy(habitBuffer[idx].uuid, (const char *)id, UUID_LENGTH - 1);
        habitBuffer[idx].uuid[UUID_LENGTH - 1] = '\0';

        strncpy(habitBuffer[idx].name, (const char *)name, MAX_NAME_SIZE - 1);
        habitBuffer[idx].name[MAX_NAME_SIZE - 1] = '\0';

        habitBuffer[idx].goal = goal;

        idx++;
    }

    if (rc != SQLITE_DONE && rc != SQLITE_ROW)
    {
        ESP_LOGE(TAG, "Error during stepping: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return idx;
}

esp_err_t HabitAddDB(sqlite3 *db, const char *uuid, const char *name, uint8_t goal_flags)
{
    const char *TAG = "habit::HabitAddDB";
    ESP_LOGI(TAG, "Free heap: %lu bytes", esp_get_free_heap_size());

    const char *sql = "INSERT INTO habits (id, name, day_goals) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Prepare failed: %s", sqlite3_errmsg(db));
        return ESP_FAIL;
    }

    sqlite3_bind_text(stmt, 1, uuid, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, (int)goal_flags);

    esp_err_t result = (sqlite3_step(stmt) == SQLITE_DONE) ? ESP_OK : ESP_FAIL;

    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to insert habit: %s", sqlite3_errmsg(db));
        ESP_LOGI(TAG, "Free heap: %lu bytes", esp_get_free_heap_size());
    }
    else
    {
        ESP_LOGI(TAG, "Added <%s> with UUID <%s>", name, uuid);
    }

    sqlite3_finalize(stmt);
    return result;
}

esp_err_t ParseHabitsJSON(sqlite3 *db, const cJSON *habitItem)
{
    const char *TAG = "habit::ParseHabitJSON";

    if (!db)
    {
        ESP_LOGE(TAG, "Invalid input: Missing database");
        return ESP_ERR_INVALID_ARG;
    }

    if (!cJSON_IsObject(habitItem))
    {
        ESP_LOGE(TAG, "Invalid JSON: 'task' should be a JSON object");
        return ESP_ERR_INVALID_ARG;
    }

    // Extract UUID
    cJSON *id = cJSON_GetObjectItem(habitItem, "id");
    if (!cJSON_IsString(id))
    {
        ESP_LOGE(TAG, "Missing or invalid 'id'");
        return ESP_ERR_INVALID_ARG;
    }

    // Extract Name
    cJSON *name = cJSON_GetObjectItem(habitItem, "name");
    if (!cJSON_IsString(name))
    {
        ESP_LOGE(TAG, "Missing or invalid 'name'");
        return ESP_ERR_INVALID_ARG;
    }

    // Check lengths
    if (strlen(id->valuestring) >= UUID_LENGTH)
    {
        ESP_LOGE(TAG, "UUID too long (max %d)", UUID_LENGTH - 1);
        return ESP_ERR_INVALID_ARG;
    }

    if (strlen(name->valuestring) >= MAX_NAME_SIZE)
    {
        ESP_LOGE(TAG, "Name too long (max %d)", MAX_NAME_SIZE - 1);
    }

    // Extract Goal Flags
    cJSON *goal_flags = cJSON_GetObjectItem(habitItem, "goal");
    if (!cJSON_IsNumber(goal_flags) || goal_flags->valueint < 0 || goal_flags->valueint > 0x7F)
    {
        ESP_LOGE(TAG, "Missing or invalid 'goal'");
        return ESP_ERR_INVALID_ARG;
    }

    // Add habit to DB
    esp_err_t result = HabitAddDB(db, id->valuestring, name->valuestring, (uint8_t)goal_flags->valueint);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add habit to DB");
    }

    return result;
}

esp_err_t HabitAddEntryDB(sqlite3 *db, const char *habit_id, time_t datetime)
{
    const char *TAG = "habit::HabitAddEntryDB";

    time_t date = datetime - (datetime % 86400); // UTC truncation

    const char *sql = "INSERT OR IGNORE INTO habit_entries (habit_id, date) VALUES (?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Prepare failed: %s", sqlite3_errmsg(db));
        return ESP_FAIL;
    }

    sqlite3_bind_text(stmt, 1, habit_id, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, date);

    esp_err_t result = (sqlite3_step(stmt) == SQLITE_DONE) ? ESP_OK : ESP_FAIL;

    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add entry: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return result;
}

esp_err_t HabitRemoveEntryDB(sqlite3 *db, const char *habit_id, time_t datetime)
{
    const char *TAG = "habit::HabitRemoveEntryDB";

    time_t date = datetime - (datetime % 86400); // UTC truncation

    const char *sql = "DELETE FROM habit_entries WHERE habit_id = ? AND date = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Prepare failed: %s", sqlite3_errmsg(db));
        return ESP_FAIL;
    }

    sqlite3_bind_text(stmt, 1, habit_id, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, date);

    esp_err_t result = (sqlite3_step(stmt) == SQLITE_DONE) ? ESP_OK : ESP_FAIL;

    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to remove entry: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return result;
}

esp_err_t HabitRetrieveWeekCompletionDB(sqlite3 *db, habit_t *habit, time_t date)
{
    const char *TAG = "habit::HabitRetrieveWeekCompletionDB";

    // Check if day should be done
    {
        struct tm day_tm;
        if (!localtime_r(&date, &day_tm))
        {
            ESP_LOGE(TAG, "Invalid datetime parameter!");
            return ESP_ERR_INVALID_ARG;
        }

        // Check if this day should be done
        for (int i = 0; i < 7; i++)
        {
            // I love bit operators
            // Checks for day of the week i days ago and determines if that day was due
            habit->completed[i] = ((habit->goal >> (6 - (day_tm.tm_wday - i))) & 1) ? 2 : 0;
        }
    }

    time_t today = date - (date % 86400); // UTC truncation

    sqlite3_stmt *stmt;
    const char *sql = "SELECT date FROM habit_entries WHERE habit_id = ? AND date IN (?, ?, ?, ?, ?, ?, ?)";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Prepare failed: %s", sqlite3_errmsg(db));
        return ESP_FAIL;
    }

    for (int i = 0; i < 7; i++)
    {
        // Insert today and 6 prior days to test
        sqlite3_bind_int64(stmt, i + 1, today - i * 86400);
    }

    // Check which days are valid
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        time_t found_day = sqlite3_column_int64(stmt, 0);

        for (int i = 0; i < 7; i++)
        {
            if ((today - i * 86400) == found_day)
            {
                habit->completed[i] = 1;
                break;
            }
        }
    }
    sqlite3_finalize(stmt);
    return ESP_OK;
}

#include "habit.h"
#include "esp_log.h"

esp_err_t HabitAddDB(sqlite3 *db, const char *uuid, const char *name)
{
    static const char *TAG = "habit::HabitAddDB";

    const char *sql = "INSERT INTO habits (id, name) VALUES (?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Prepare failed: %s", sqlite3_errmsg(db));
        return ESP_FAIL;
    }

    sqlite3_bind_text(stmt, 1, uuid, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, name, -1, SQLITE_STATIC);

    esp_err_t result = (sqlite3_step(stmt) == SQLITE_DONE) ? ESP_OK : ESP_FAIL;

    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to insert habit: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return result;
}

esp_err_t HabitAddEntryDB(sqlite3 *db, const char *habit_id, const char *date)
{
    static const char *TAG = "habit::HabitAddEntryDB";

    const char *sql = "INSERT OR IGNORE INTO habit_entries (habit_id, date) VALUES (?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Prepare failed: %s", sqlite3_errmsg(db));
        return ESP_FAIL;
    }

    sqlite3_bind_text(stmt, 1, habit_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, date, -1, SQLITE_STATIC);

    esp_err_t result = (sqlite3_step(stmt) == SQLITE_DONE) ? ESP_OK : ESP_FAIL;

    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add entry: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return result;
}

esp_err_t HabitRemoveEntryDB(sqlite3 *db, const char *habit_id, const char *date)
{
    static const char *TAG = "habit::HabitRemoveEntryDB";

    const char *sql = "DELETE FROM habit_entries WHERE habit_id = ? AND date = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Prepare failed: %s", sqlite3_errmsg(db));
        return ESP_FAIL;
    }

    sqlite3_bind_text(stmt, 1, habit_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, date, -1, SQLITE_STATIC);

    esp_err_t result = (sqlite3_step(stmt) == SQLITE_DONE) ? ESP_OK : ESP_FAIL;

    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to remove entry: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return result;
}

int HabitCheckEntryDB(sqlite3 *db, const char *habit_id, const char *date)
{
    static const char *TAG = "habit::HabitCheckEntryDB";

    const char *sql = "SELECT 1 FROM habit_entries WHERE habit_id = ? AND date = ? LIMIT 1;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Prepare failed: %s", sqlite3_errmsg(db));
        return ESP_FAIL;
    }

    sqlite3_bind_text(stmt, 1, habit_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, date, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_ROW);
}

// ----------------------------------------- TEST SCRIPT ------------------------------------------
// TODO: Test script
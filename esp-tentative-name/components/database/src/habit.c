#include "habit.h"
#include "esp_log.h"
#include <cJSON.h>
#include <string.h>

uint8_t GetDayFlag(int tm_wday);

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

        if (!id || !name)
        {
            ESP_LOGW(TAG, "Null field detected at row %d", idx);
            continue;
        }

        strncpy(habitBuffer[idx].uuid, (const char *)id, UUID_LENGTH - 1);
        habitBuffer[idx].uuid[UUID_LENGTH - 1] = '\0';

        strncpy(habitBuffer[idx].name, (const char *)name, MAX_NAME_SIZE - 1);
        habitBuffer[idx].name[MAX_NAME_SIZE - 1] = '\0';

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
    }

    sqlite3_finalize(stmt);
    ESP_LOGI(TAG, "Added <%s> with UUID <%s>", name, uuid);
    return result;
}

esp_err_t ParseHabitsJSON(sqlite3 *db, const char *json)
{
    const char *TAG = "habit::ParseHabitJSON";

    if (!db || !json)
    {
        ESP_LOGE(TAG, "Invalid input: db or json is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    cJSON *root = cJSON_Parse(json);
    if (!root)
    {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return ESP_FAIL;
    }

    cJSON *habit = cJSON_GetObjectItem(root, "habit");
    if (!cJSON_IsObject(habit))
    {
        ESP_LOGE(TAG, "Missing or invalid 'habit' object");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    // Extract UUID
    cJSON *id = cJSON_GetObjectItem(habit, "id");
    if (!cJSON_IsString(id))
    {
        ESP_LOGE(TAG, "Missing or invalid 'id'");
        cJSON_Delete(root);
        return ESP_ERR_INVALID_ARG;
    }

    // Extract Name
    cJSON *name = cJSON_GetObjectItem(habit, "name");
    if (!cJSON_IsString(name))
    {
        ESP_LOGE(TAG, "Missing or invalid 'name'");
        cJSON_Delete(root);
        return ESP_ERR_INVALID_ARG;
    }

    // Extract Goal Flags
    cJSON *goal_flags = cJSON_GetObjectItem(habit, "goal");
    if (!cJSON_IsNumber(goal_flags) || goal_flags->valueint < 0 || goal_flags->valueint > 0x7F)
    {
        ESP_LOGE(TAG, "Missing or invalid 'goal'");
        cJSON_Delete(root);
        return ESP_ERR_INVALID_ARG;
    }

    // Add habit to DB
    esp_err_t result = HabitAddDB(db, id->valuestring, name->valuestring, (uint8_t)goal_flags->valueint);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add habit to DB");
    }

    cJSON_Delete(root);
    return result;
}

esp_err_t HabitAddEntryDB(sqlite3 *db, const char *habit_id, time_t datetime)
{
    const char *TAG = "habit::HabitAddEntryDB";

    // Convert time to date
    struct tm day_tm;
    if (!localtime_r(&datetime, &day_tm))
    {
        ESP_LOGE(TAG, "Invalid datetime parameter!");
        return -1;
    }
    char date[11];
    strftime(date, sizeof(date), "%Y-%m-%d", &day_tm);

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

esp_err_t HabitRemoveEntryDB(sqlite3 *db, const char *habit_id, time_t datetime)
{
    const char *TAG = "habit::HabitRemoveEntryDB";

    // Convert time to date
    struct tm day_tm;
    if (!localtime_r(&datetime, &day_tm))
    {
        ESP_LOGE(TAG, "Invalid datetime parameter!");
        return -1;
    }
    char date[11];
    strftime(date, sizeof(date), "%Y-%m-%d", &day_tm);

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

int HabitEntryCompletedDB(sqlite3 *db, const char *habit_id, time_t datetime)
{
    const char *TAG = "habit::HabitCheckEntryDB";

    // Convert time to date
    struct tm day_tm;
    if (!localtime_r(&datetime, &day_tm))
    {
        ESP_LOGE(TAG, "Invalid datetime parameter!");
        return -1;
    }
    char date[11];
    strftime(date, sizeof(date), "%Y-%m-%d", &day_tm);

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

int HabitEntryDueDB(sqlite3 *db, const char *habit_id, time_t datetime)
{
    const char *TAG = "habit::HabitCheckDueEntryDB";

    struct tm day_tm;
    if (!localtime_r(&datetime, &day_tm))
    {
        ESP_LOGE(TAG, "Invalid datetime parameter!");
        return -1;
    }

    const char *sql = "SELECT day_goals FROM habits WHERE id = ?";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Prepare failed: %s", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, habit_id, -1, SQLITE_STATIC);

    int day_goal = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        day_goal = sqlite3_column_int(stmt, 0);
    }
    else
    {
        ESP_LOGE(TAG, "Habit with id: %s not found!", habit_id);
        return -1;
    }

    sqlite3_finalize(stmt);

    // Check if today is a valid day
    uint8_t today_flag = GetDayFlag(day_tm.tm_wday);
    return (day_goal & today_flag) != 0;
}

uint8_t GetDayFlag(int tm_wday)
{
    switch (tm_wday)
    {
    case 0:
        return SUNDAY_FLAG;
    case 1:
        return MONDAY_FLAG;
    case 2:
        return TUESDAY_FLAG;
    case 3:
        return WEDNESDAY_FLAG;
    case 4:
        return THURSDAY_FLAG;
    case 5:
        return FRIDAY_FLAG;
    case 6:
        return SATURDAY_FLAG;
    default:
        return 0;
    }
}

// ----------------------------------------- TEST SCRIPT ------------------------------------------
esp_err_t TestHabitFunctions(sqlite3 *db)
{
    const char *TAG = "habit::TestHabitFunctions";

    time_t now = time(NULL);
    struct tm tm_day;

    // Generate two test days: today and yesterday
    time_t date_today = now;
    time_t date_yesterday = now - 86400;

    const char *TEST_UUID = "123e4567-e89b-12d3-a456-426614174000";
    const char *TEST_NAME = "Touch Grass";
    int TEST_GOAL_MASK = 0b0111110;

    ESP_LOGI(TAG, "=== Testing Habit Functions ===");

    ESP_LOGI(TAG, "Adding test habit...");
    if (HabitAddDB(db, TEST_UUID, TEST_NAME, TEST_GOAL_MASK) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add habit table");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Adding entry for today...");
    if (HabitAddEntryDB(db, TEST_UUID, date_today) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add today's entry");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Adding entry for yesterday...");
    if (HabitAddEntryDB(db, TEST_UUID, date_yesterday) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add yesterday's entry");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Checking if today's entry exists...");
    int result = HabitEntryCompletedDB(db, TEST_UUID, date_today);
    if (result != 1)
    {
        ESP_LOGE(TAG, "Expected today's entry to exist, got %d", result);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Checking if yesterday's entry exists...");
    result = HabitEntryCompletedDB(db, TEST_UUID, date_yesterday);
    if (result != 1)
    {
        ESP_LOGE(TAG, "Expected yesterday's entry to exist, got %d", result);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Removing yesterday's entry...");
    if (HabitRemoveEntryDB(db, TEST_UUID, date_yesterday) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to remove yesterday's entry");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Verifying yesterday's entry is removed...");
    result = HabitEntryCompletedDB(db, TEST_UUID, date_yesterday);
    if (result != 0)
    {
        ESP_LOGE(TAG, "Expected yesterday's entry to be gone, got %d", result);
        return ESP_FAIL;
    }

    // Check if task is due today based on goal mask
    localtime_r(&date_today, &tm_day);
    int due = HabitEntryDueDB(db, TEST_UUID, date_today);

    if ((TEST_GOAL_MASK >> tm_day.tm_wday) & 1)
    {
        if (!due)
        {
            ESP_LOGE(TAG, "Habit is due today but HabitEntryDueDB returned false");
            return ESP_FAIL;
        }
    }
    else
    {
        if (due)
        {
            ESP_LOGE(TAG, "Habit is not due today but HabitEntryDueDB returned true");
            return ESP_FAIL;
        }
    }

    ESP_LOGI(TAG, "=== All habit DB tests passed! ===");
    return ESP_OK;
}
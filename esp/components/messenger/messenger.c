#include "messenger.h"
#include "freertos/FreeRTOS.h"
#include "mqtt_conn.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <cJSON.h>

#define MAX_RETRIES 5
#define MQTT_LOOP_TIMEOUT_MS 5000

#define PATH_LENGTH 255 + 32 + UUID_LENGTH

// ------------------------------------------ Tasks -----------------------------------------------

FILE *fopen_mkdir(const char *path, const char *mode)
{
    char *p = strdup(path);
    char *sep = strchr(p + 1, '/');
    while (sep != NULL)
    {
        *sep = '\0';
        if (mkdir(p, 0777) && errno != EEXIST)
        {
            fprintf(stderr, "error while trying to create %s\n", p);
        }
        *sep = '/';
        sep = strchr(sep + 1, '/');
    }
    free(p);
    return fopen(path, mode);
}

esp_err_t UpdateTaskStatus(const char *uuid, TASK_STATUS new_status)
{
    static const char *TAG = "messenger::UpdateTaskStatus";

    // --- Update database ---
    esp_err_t ret = UpdateTaskStatusDB(uuid, new_status);
    if (ret != ESP_OK)
    {
        return ret;
    }

    // --- Write to disk ---

    // Set path name to UUID to be changed, this both makes the file contents apparent,
    // but also results put in the new requests file will be overwritten.
    char path[PATH_LENGTH];
    snprintf(path, PATH_LENGTH, MOUNT_POINT TASK_REQUESTS_DIR "/%s.txt", uuid); // Ensure full path

    FILE *file = fopen_mkdir(path, "w");
    if (!file)
    {
        ESP_LOGE(TAG, "Failed to open file %s for writing: %s", path, strerror(errno));
        return ESP_FAIL;
    }

    fprintf(file, "%d\n", new_status);
    fclose(file);
    ESP_LOGI(TAG, "Wrote request: %s with status %d", uuid, new_status);
    return ESP_OK;
}

esp_err_t UploadTaskRequests(struct callback_data_t *cb_data, const char *device_id)
{
    static const char *TAG = "messenger::UploadTaskRequests";

    DIR *dir = opendir(MOUNT_POINT TASK_REQUESTS_DIR);
    if (!dir)
    {
        ESP_LOGE(TAG, "Failed to open directory: %s", TASK_REQUESTS_DIR);
        return ESP_FAIL;
    }

    struct dirent *entry;
    char filepaths[MAX_ENTRIES][PATH_LENGTH];
    int count = 0;

    // Prepare root object and "tasks" array
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "id", device_id);
    cJSON_AddStringToObject(root, "action", "update");
    cJSON_AddStringToObject(root, "type", "task");
    cJSON *tasks = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "task", tasks);

    // Read request files
    while ((entry = readdir(dir)) && count < MAX_ENTRIES)
    {
        if (entry->d_type != DT_REG)
            continue;

        char path[PATH_LENGTH];
        snprintf(path, PATH_LENGTH, MOUNT_POINT TASK_REQUESTS_DIR "/%s", entry->d_name);

        FILE *file = fopen(path, "r");
        if (!file)
        {
            ESP_LOGW(TAG, "Failed to open %s", path);
            continue;
        }

        int status;
        if (fscanf(file, "%d", &status) != 1)
        {
            ESP_LOGW(TAG, "Invalid status in file: %s", path);
            fclose(file);
            continue;
        }
        fclose(file);

        // Extract UUID from filename (strip ".txt")
        char uuid[128];
        strncpy(uuid, entry->d_name, sizeof(uuid));
        char *ext = strrchr(uuid, '.');
        if (ext)
            *ext = '\0';

        cJSON *task = cJSON_CreateObject();
        cJSON_AddStringToObject(task, "id", uuid);
        cJSON_AddNumberToObject(task, "completion", status);
        cJSON_AddItemToArray(tasks, task);

        strncpy(filepaths[count], path, sizeof(filepaths[count]));
        count++;
    }

    closedir(dir);

    if (count == 0)
    {
        cJSON_Delete(root);
        ESP_LOGI(TAG, "No request files to sync.");
        return ESP_OK;
    }

    // Publish to server
    char *msg = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    mqtt_publish(msg, strlen(msg));
    size_t i = 0;
    for (; i < MAX_RETRIES; i++)
    {
        mqtt_loop(MQTT_LOOP_TIMEOUT_MS);
        if (cb_data->update_ack == 0)
        {
            ESP_LOGI(TAG, "No ack received, retrying...");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            mqtt_publish(msg, strlen(msg));
        }
        else
        {
            ESP_LOGI(TAG, "Server acknowledged the request batch.");
            break;
        }
    }

    free(msg);

    if (i < MAX_RETRIES)
    {
        for (int j = 0; j < count; j++)
        {
            if (remove(filepaths[j]) != 0)
            {
                ESP_LOGW(TAG, "Failed to delete %s", filepaths[j]);
            }
            else
            {
                ESP_LOGI(TAG, "Deleted: %s", filepaths[j]);
            }
        }
        return ESP_OK;
    }

    ESP_LOGE(TAG, "Failed to get server acknowledgment after %d retries.", MAX_RETRIES);
    return ESP_FAIL;
}

// ------------------------------------------ Habits ----------------------------------------------

esp_err_t HabitAddEntry(const char *habit_id, time_t datetime)
{
    static const char *TAG = "messenger::HabitAddEntry";

    time_t date = datetime - (datetime % 86400); // UTC truncation

    esp_err_t rc = HabitAddEntryDB(habit_id, date);
    if (rc != ESP_OK)
    {
        return rc;
    }

    // --- Write to disk ---

    // Set path name to UUID to be changed, this both makes the file contents apparent,
    // but also results put in the new requests file will be overwritten.
    char path[PATH_LENGTH];
    snprintf(path, PATH_LENGTH, MOUNT_POINT HABIT_REQUESTS_DIR "/%s-%lld.txt", habit_id, datetime); // Ensure full path

    FILE *file = fopen_mkdir(path, "w");
    if (!file)
    {
        ESP_LOGE(TAG, "Failed to open file %s for writing: %s", path, strerror(errno));
        return ESP_FAIL;
    }

    fprintf(file, "add\n");
    fclose(file);
    ESP_LOGI(TAG, "Wrote request: %s with datetime %lld to add", habit_id, date);
    return ESP_OK;
}

esp_err_t HabitRemoveEntry(const char *habit_id, time_t datetime)
{
    static const char *TAG = "messenger::HabitRemoveEntry";

    time_t date = datetime - (datetime % 86400); // UTC truncation

    esp_err_t rc = HabitRemoveEntryDB(habit_id, date);
    if (rc != ESP_OK)
    {
        return rc;
    }

    // --- Write to disk ---

    // Set path name to UUID to be changed, this both makes the file contents apparent,
    // but also results put in the new requests file will be overwritten.
    char path[PATH_LENGTH];
    snprintf(path, PATH_LENGTH, MOUNT_POINT HABIT_REQUESTS_DIR "/%s-%lld.txt", habit_id, datetime); // Ensure full path

    FILE *file = fopen_mkdir(path, "w");
    if (!file)
    {
        ESP_LOGE(TAG, "Failed to open file %s for writing: %s", path, strerror(errno));
        return ESP_FAIL;
    }

    fprintf(file, "del\n");
    fclose(file);
    ESP_LOGI(TAG, "Wrote request: %s with datetime %lld to delete", habit_id, date);
    return ESP_OK;
}

esp_err_t UploadHabitRequests(struct callback_data_t *cb_data, const char *device_id)
{
    static const char *TAG = "messenger::UploadHabitRequests";

    DIR *dir = opendir(MOUNT_POINT HABIT_REQUESTS_DIR);
    if (!dir)
    {
        ESP_LOGE(TAG, "Failed to open directory: %s", HABIT_REQUESTS_DIR);
        return ESP_FAIL;
    }

    struct dirent *entry;
    char filepaths[MAX_ENTRIES][PATH_LENGTH];
    int count = 0;

    // Prepare root object and "tasks" array
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "id", device_id);
    cJSON_AddStringToObject(root, "action", "update");
    cJSON_AddStringToObject(root, "type", "habit");
    cJSON *entries = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "habit", entries);

    // Read request files
    while ((entry = readdir(dir)) && count < MAX_ENTRIES)
    {
        if (entry->d_type != DT_REG)
            continue;

        char path[PATH_LENGTH];
        snprintf(path, PATH_LENGTH, MOUNT_POINT HABIT_REQUESTS_DIR "/%s", entry->d_name);

        FILE *file = fopen(path, "r");
        if (!file)
        {
            ESP_LOGW(TAG, "Failed to open %s", path);
            continue;
        }

        char action[3];
        if (fscanf(file, "%s", action) != 1)
        {
            ESP_LOGW(TAG, "Invalid status in file: %s", path);
            fclose(file);
            continue;
        }
        fclose(file);

        // Extract UUID from filename
        char uuid[128];
        time_t date;
        {
            char temp[128];

            strncpy(temp, entry->d_name, sizeof(temp));
            // Parse: we assume format "uuid_timestamp"
            if (sscanf(temp, "%[^_]_%lld", uuid, &date) == 2)
            {
                ESP_LOGI(TAG, "UUID: %s\n", uuid);
                ESP_LOGI(TAG, "Timestamp: %lld\n", date);
            }
            else
            {
                ESP_LOGE(TAG, "Filename format invalid.\n");
            }
        }

        cJSON *habit_entry = cJSON_CreateObject();
        cJSON_AddStringToObject(habit_entry, "id", uuid);
        cJSON_AddNumberToObject(habit_entry, "date", date);
        cJSON_AddStringToObject(habit_entry, "action", action);
        cJSON_AddItemToArray(entries, habit_entry);

        strncpy(filepaths[count], path, sizeof(filepaths[count]));
        count++;
    }

    closedir(dir);

    if (count == 0)
    {
        cJSON_Delete(root);
        ESP_LOGI(TAG, "No request files to sync.");
        return ESP_OK;
    }

    // Publish to server
    char *msg = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    mqtt_publish(msg, strlen(msg));
    size_t i = 0;
    for (; i < MAX_RETRIES; i++)
    {
        mqtt_loop(MQTT_LOOP_TIMEOUT_MS);
        if (cb_data->update_ack == 0)
        {
            ESP_LOGI(TAG, "No ack received, retrying...");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            mqtt_publish(msg, strlen(msg));
        }
        else
        {
            ESP_LOGI(TAG, "Server acknowledged the request batch.");
            break;
        }
    }

    free(msg);

    if (i < MAX_RETRIES)
    {
        for (int j = 0; j < count; j++)
        {
            if (remove(filepaths[j]) != 0)
            {
                ESP_LOGW(TAG, "Failed to delete %s", filepaths[j]);
            }
            else
            {
                ESP_LOGI(TAG, "Deleted: %s", filepaths[j]);
            }
        }
        return ESP_OK;
    }

    ESP_LOGE(TAG, "Failed to get server acknowledgment after %d retries.", MAX_RETRIES);
    return ESP_FAIL;
}

// ------------------------------------------ Events ----------------------------------------------

esp_err_t RemoveEvent(const char *uuid)
{
    static const char *TAG = "messenger::RemoveEvent";

    // --- Update database ---
    esp_err_t ret = RemoveEventDB(uuid);
    if (ret != ESP_OK)
    {
        return ret;
    }

    // --- Write to disk ---

    // Set path name to UUID to be changed, this both makes the file contents apparent,
    // but also results put in the new requests file will be overwritten.
    char path[PATH_LENGTH];
    snprintf(path, PATH_LENGTH, MOUNT_POINT EVENT_REQUESTS_DIR "/%s.txt", uuid); // Ensure full path

    FILE *file = fopen_mkdir(path, "w");
    if (!file)
    {
        ESP_LOGE(TAG, "Failed to open file %s for writing: %s", path, strerror(errno));
        return ESP_FAIL;
    }

    fclose(file);
    ESP_LOGI(TAG, "Wrote deletion request for: %s", uuid);
    return ESP_OK;
}

esp_err_t UploadEventRequests(struct callback_data_t *cb_data, const char *device_id)
{
    static const char *TAG = "messenger::UploadTaskRequests";

    DIR *dir = opendir(MOUNT_POINT TASK_REQUESTS_DIR);
    if (!dir)
    {
        ESP_LOGE(TAG, "Failed to open directory: %s", TASK_REQUESTS_DIR);
        return ESP_FAIL;
    }

    struct dirent *entry;
    char filepaths[MAX_ENTRIES][PATH_LENGTH];
    int count = 0;

    // Prepare root object and "tasks" array
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "id", device_id);
    cJSON_AddStringToObject(root, "action", "update");
    cJSON_AddStringToObject(root, "type", "event");
    cJSON *events = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "event", events);

    // Read request files
    while ((entry = readdir(dir)) && count < MAX_ENTRIES)
    {
        if (entry->d_type != DT_REG)
            continue;

        char path[PATH_LENGTH];
        snprintf(path, PATH_LENGTH, MOUNT_POINT EVENT_REQUESTS_DIR "/%s", entry->d_name);

        // Extract UUID from filename (strip ".txt")
        char uuid[128];
        strncpy(uuid, entry->d_name, sizeof(uuid));
        char *ext = strrchr(uuid, '.');
        if (ext)
            *ext = '\0';

        cJSON *event = cJSON_CreateObject();
        cJSON_AddStringToObject(event, "id", uuid);
        cJSON_AddItemToArray(events, event);

        strncpy(filepaths[count], path, sizeof(filepaths[count]));
        count++;
    }

    closedir(dir);

    if (count == 0)
    {
        cJSON_Delete(root);
        ESP_LOGI(TAG, "No request files to sync.");
        return ESP_OK;
    }

    // Publish to server
    char *msg = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    mqtt_publish(msg, strlen(msg));
    size_t i = 0;
    for (; i < MAX_RETRIES; i++)
    {
        mqtt_loop(MQTT_LOOP_TIMEOUT_MS);
        if (cb_data->update_ack == 0)
        {
            ESP_LOGI(TAG, "No ack received, retrying...");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            mqtt_publish(msg, strlen(msg));
        }
        else
        {
            ESP_LOGI(TAG, "Server acknowledged the request batch.");
            break;
        }
    }

    free(msg);

    if (i < MAX_RETRIES)
    {
        for (int j = 0; j < count; j++)
        {
            if (remove(filepaths[j]) != 0)
            {
                ESP_LOGW(TAG, "Failed to delete %s", filepaths[j]);
            }
            else
            {
                ESP_LOGI(TAG, "Deleted: %s", filepaths[j]);
            }
        }
        return ESP_OK;
    }

    ESP_LOGE(TAG, "Failed to get server acknowledgment after %d retries.", MAX_RETRIES);
    return ESP_FAIL;
}
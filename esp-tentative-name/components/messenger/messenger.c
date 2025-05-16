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

FILE * fopen_mkdir(const char *path, const char *mode)
{
    char *p = strdup(path);
    char *sep = strchr(p+1, '/');
    while(sep != NULL)
    {
        *sep = '\0';
        if (mkdir(p, 0777) && errno != EEXIST)
        {
            fprintf(stderr, "error while trying to create %s\n", p);
        }
        *sep = '/';
        sep = strchr(sep+1, '/');
    }
    free(p);
    return fopen(path, mode);
}

esp_err_t UpdateTaskStatus(sqlite3 *db, const char *uuid, TASK_STATUS new_status)
{
    static const char *TAG = "sender::UpdateTaskStatus";

    // --- Update database ---
    esp_err_t ret = UpdateTaskStatusDB(db, uuid, new_status);
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

esp_err_t SyncTaskRequests(struct callback_data_t *cb_data, const char *device_id)
{
    static const char *TAG = "sender::UpdateTaskStatus";

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
    cJSON *tasks = cJSON_CreateArray();

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

    mqtt_connect();
    mqtt_subscribe();
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

    mqtt_unsubscribe();
    mqtt_disconnect();
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

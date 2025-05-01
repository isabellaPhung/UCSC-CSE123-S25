#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "mqtt_conn.h"

#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "cJSON.h"
#include "esp_log.h"

#include "SQL.h"
#include "task.h" // Your task DB header
#include "sqlite3.h"

static const char *TAG = "MQTT_TASK_APP";

struct callback_data
{
    int expected;
    int cur_index;
    sqlite3 *db;
};

void demo_callback(const char *payload, size_t payload_length, void *cb_data)
{
    struct callback_data *data = (struct callback_data *)cb_data;

    // Copy payload into a null-terminated buffer
    char *json_buf = calloc(payload_length + 1, sizeof(char));
    if (!json_buf)
    {
        ESP_LOGE(TAG, "Failed to allocate buffer for JSON");
        return;
    }
    memcpy(json_buf, payload, payload_length);

    // If it's task data, parse into database
    if (strstr(json_buf, "\"task\"") != NULL)
    {
        ESP_LOGI(TAG, "Received task JSON from MQTT");
        if (ParseTasksJSON(data->db, json_buf) != 0)
        {
            ESP_LOGE(TAG, "Failed to parse task JSON");
        }
        else
        {
            ESP_LOGI(TAG, "Successfully added tasks from JSON");
        }
    }

    // Parse control messages from server
    cJSON *root = cJSON_Parse(json_buf);
    if (root)
    {
        cJSON *item = cJSON_GetObjectItem(root, "id");
        if (item && strcmp(item->valuestring, "server") == 0)
        {
            ESP_LOGI(TAG, "Payload comes from server");
            item = cJSON_GetObjectItem(root, "action");
            if (item && strcmp(item->valuestring, "length") == 0)
            {
                item = cJSON_GetObjectItem(root, "length");
                if (item && cJSON_IsNumber(item))
                {
                    data->expected = item->valueint;
                    data->cur_index = 0;
                    ESP_LOGI(TAG, "Expecting %d tasks", data->expected);
                }
            }
            else if (item && strcmp(item->valuestring, "response") == 0)
            {
                ESP_LOGI(TAG, "Server response index %d", data->cur_index);
                data->cur_index++;
            }
        }
        cJSON_Delete(root);
    }

    free(json_buf);
    ESP_LOGI(TAG, "MQTT callback done");
}

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());

    // Initialize SQLite3 database
    sqlite3 *db = NULL;
    if (InitSQL(&db) != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize SQLite database");
        return;
    }

    // Initialize callback data
    struct callback_data cb_data = {
        .expected = 0,
        .cur_index = -1,
        .db = db};

    // Setup MQTT and publish initial request
    int return_status = mqtt_init(&demo_callback, (void *)&cb_data);
    mqtt_connect();
    mqtt_subscribe();

    // Request task list from server
    const char *payload = "{\"id\":\"c72572d0-8c8c-4f37-8ff6-829cac2eabec\",\"action\":\"refresh\"}";
    mqtt_publish(payload, strlen(payload));

    // Wait for expected task responses
    while (1)
    {
        mqtt_loop(5000);

        if (cb_data.expected == 0)
        {
            ESP_LOGI(TAG, "No length message yet. Re-requesting after delay...");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            mqtt_publish(payload, strlen(payload));
        }
        else if (cb_data.expected != cb_data.cur_index)
        {
            ESP_LOGI(TAG, "Still waiting for remaining tasks (%d/%d)...", cb_data.cur_index, cb_data.expected);
        }
        else
        {
            ESP_LOGI(TAG, "All expected tasks received!");
            break;
        }
    }

    mqtt_unsubscribe();
    mqtt_disconnect();

    Task tasks[3] = {0};
    RetrieveTasksSortedDB(db, tasks, 3);
    for (size_t i = 0; i < 3; i++)
    {
        PrintTask(tasks[i]);
    }

    ESP_LOGI(TAG, "Demo finished. Return status: %d", return_status);

    // Optional: close DB (up to you when to persist or flush it)
    if (db)
    {
        CloseSQL(&db);
    }
}

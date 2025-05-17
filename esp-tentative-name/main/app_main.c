#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "mqtt_conn.h"

#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "wifi_setup.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "pcf8523.h"

#include "messenger.h"
#include "database.h"

#include "display_init.h" //initalizes LVGL and display hardware
#include "helper_menus.h"

#include "cJSON.h"

#include "esp_log.h"

static const char *TAG = "MAIN";

void callback(const char *payload, size_t payload_length, void *cb_data)
{
    struct callback_data_t *data = (struct callback_data_t *)cb_data;
    cJSON *root = cJSON_ParseWithLength(payload, payload_length);
    cJSON *item;
    item = cJSON_GetObjectItem(root, "id");
    if (item && (strcmp(item->valuestring, "server") == 0))
    {
        item = cJSON_GetObjectItem(root, "action");
        if (item && (strcmp(item->valuestring, "length") == 0))
        {
            item = cJSON_GetObjectItem(root, "length");
            if (item && cJSON_IsNumber(item))
            {
                data->expected = item->valueint;
                data->cur_index = 0;
                ESP_LOGI(TAG, "Expecting %d tasks", data->expected);
            }
        }
        else if (item && (strcmp(item->valuestring, "response") == 0))
        {
            ESP_LOGI(TAG, "Server response index %d", data->cur_index);
            cJSON *task = cJSON_GetObjectItem(root, "task");
            cJSON *event = cJSON_GetObjectItem(root, "event");
            cJSON *habit = cJSON_GetObjectItem(root, "habit");
            if (task)
            {
                // Add task information to database
                ParseTasksJSON(data->db_ptr, task);
            }
            else if (event)
            {
                ParseEventsJSON(data->db_ptr, event);
            }
            else if (habit)
            {
                ParseHabitsJSON(data->db_ptr, habit);
            }
            else
            {
                ESP_LOGE(TAG, "Missing entry information!");
            }

            data->cur_index++;
        }
        else if (item && (strcmp(item->valuestring, "ack") == 0))
        {
            ESP_LOGI(TAG, "Server acknowledged update!");
            data->update_ack = 1;
        }
    }
    cJSON_Delete(root);
    ESP_LOGI(TAG, "Callback function called\n");
}

#define DEVICE_ID "55"
#define RETRY_DELAY_MS 5000U

int request_backup(struct callback_data_t *cb_data)
{
    int return_status = EXIT_SUCCESS;
    size_t retries = 4;

    // Request payloads for all 3 entry types
    static const char *backup_payload[3] = {"{\"id\":\"" DEVICE_ID "\",\"action\":\"refresh\",\"type\":\"task\"}",
                                            "{\"id\":\"" DEVICE_ID "\",\"action\":\"refresh\",\"type\":\"event\"}",
                                            "{\"id\":\"" DEVICE_ID "\",\"action\":\"refresh\",\"type\":\"habit\"}"};

    for (int ent_itr = 0; ent_itr < 3; ent_itr++)
    {
        mqtt_publish(backup_payload[ent_itr], strlen(backup_payload[ent_itr]));
        while (1)
        {
            mqtt_loop(5000);
            retries--;
            if (retries == 0)
            {
                return_status = EXIT_FAILURE;
                ESP_LOGW(TAG, "Three failures of backup request, trying again later");
                break;
            }
            if (cb_data->expected == 0)
            {
                ESP_LOGI(TAG, "Did not get back a length message from the server, re-publishing after a delay");
                vTaskDelay(RETRY_DELAY_MS / portTICK_PERIOD_MS);
                mqtt_publish(backup_payload[ent_itr], strlen(backup_payload[ent_itr]));
            }
            else if (cb_data->expected != cb_data->cur_index)
            {
                ESP_LOGI(TAG, "Did not get the expected amount, listening for some more time");
            }
            else
            {
                break;
            }
        }
    }

    mqtt_unsubscribe();
    mqtt_disconnect();
    return return_status;
}

int sync_database(struct callback_data_t* cb_data)
{
    // Establish connection
    if (mqtt_connect() != EXIT_SUCCESS)
    {
        ESP_LOGE(TAG, "MQTT connect failed. Aborting backup request.");
        return EXIT_FAILURE;
    }

    if (mqtt_subscribe() != EXIT_SUCCESS)
    {
        ESP_LOGE(TAG, "MQTT subscribe failed. Disconnecting and aborting.");
        mqtt_disconnect();
        return EXIT_FAILURE;
    }
    // Send outgoing requests
    UploadTaskRequests(cb_data, DEVICE_ID);

    // Populate database
    ESP_LOGI("main::Initialize LCD", "Largest free block seen by request_backup: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
    request_backup(cb_data);

    // Disconnect
    mqtt_unsubscribe();
    mqtt_disconnect();

    return EXIT_SUCCESS;
}

void app_main()
{

    // ------------------------------------- Set Up Wifi ------------------------------------------
    ESP_LOGI("main::Setting up Wifi", "Free heap total: %lu bytes", esp_get_free_heap_size());
    esp_log_level_set("*", ESP_LOG_INFO);

    esp_log_level_set("httpd_uri", ESP_LOG_ERROR);
    esp_log_level_set("httpd_txrx", ESP_LOG_ERROR);
    esp_log_level_set("httpd_parse", ESP_LOG_ERROR);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(nvs_flash_init());

    setup_wifi();

    // -------------------------------- Create new database object --------------------------------
    ESP_LOGI("main::Create new database object", "Free heap total: %lu bytes", esp_get_free_heap_size());

    sqlite3 *db;
    int db_res = InitSQL(&db);
    if (db_res != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize database!");
        return;
    }

    // ----------------------------------- Create Task ------------------------------------
    ESP_LOGI(TAG, "Adding a Task...");
    // Define a task

    // current time
    time_t t = time(NULL);

    task_t newTask = {
        .uuid = "12345",
        .name = "Capstone Project",
        .description = "Complete Capstone Project",
        .completion = INCOMPLETE,
        .priority = 5,
        .time = t,
    };

    // Add task
    int rc = AddTaskDB(db, &newTask);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to add task!");
        return;
    }
    ESP_LOGI(TAG, "Created Task!\n");

    // -------------------------------- Configure Clock (PCF8523) ---------------------------------
    ESP_LOGI("main::Configure Clock", "Free heap total: %lu bytes", esp_get_free_heap_size());

    // Establish R2C connection
    ESP_ERROR_CHECK(InitRTC());
    ESP_ERROR_CHECK(RebootRTC()); // RECONFIGURE RTC configuration (optional)

    // Get the current time from NTP server
    SetTime();

    // ------------------------------- Initialize Server Connection -------------------------------
    ESP_LOGI("main::Initialize Server Connection", "Free heap total: %lu bytes", esp_get_free_heap_size());

    struct callback_data_t cb_data;
    cb_data.expected = 0;
    cb_data.cur_index = -1;
    cb_data.db_ptr = db;

    // Initialize mqtt library
    assert(mqtt_init(&callback, (void *)&cb_data) == EXIT_SUCCESS);

    sync_database(&cb_data);

    // ------------------------------------- Initialize LCD ---------------------------------------
    ESP_LOGI("main::Initialize LCD", "Largest free block after database init: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
    ESP_LOGI("main::Initialize LCD", "Free heap total: %lu bytes", esp_get_free_heap_size());
    /* LCD HW initialization */
    ESP_ERROR_CHECK(app_lcd_init());

    /* LVGL initialization */
    ESP_ERROR_CHECK(app_lvgl_init());
    initDatabase(db);

    // --------------------------------------- Runtime --------------------------------------------
    ESP_LOGI("main::Entering Runtime", "Free heap total: %lu bytes", esp_get_free_heap_size());

    app_main_display();

    long int frame_timer = 0;
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        lv_timer_handler(); // update screen
        frame_timer++;

        // Request from server
        if (frame_timer >= 3000) // ~30 seconds
        {
            ESP_LOGI(TAG, "Preforming Server Sync!");

            sync_database(&cb_data);
            frame_timer = 0;
        }
    }

    // Close database
    CloseSQL(&db);
}

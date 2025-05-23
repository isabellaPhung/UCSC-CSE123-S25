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

int wifi_connected = false;
int RTC_updated = false;

void callback(const char *payload, size_t payload_length, void *cb_data)
{
    const char *local_tag = "main::callback";
    ESP_LOGW(local_tag, "Heap before callback: %lu", esp_get_free_heap_size());

    struct callback_data_t *data = (struct callback_data_t *)cb_data;
    data->update_ack = 0; // DEBUG: RESET THIS TO 0

    cJSON *root = cJSON_ParseWithLength(payload, payload_length);
    if (!root)
    {
        ESP_LOGE(local_tag, "Failed to parse JSON payload.");
        ESP_LOGW(local_tag, "Heap after callback: %lu", esp_get_free_heap_size());
        return;
    }

    cJSON *id = cJSON_GetObjectItem(root, "id");
    if (!id || !cJSON_IsString(id) || strcmp(id->valuestring, "server") != 0)
    {
        cJSON_Delete(root);
        ESP_LOGW(local_tag, "Heap after callback: %lu", esp_get_free_heap_size());
        return;
    }

    cJSON *action = cJSON_GetObjectItem(root, "action");
    if (!action || !cJSON_IsString(action))
    {
        cJSON_Delete(root);
        ESP_LOGW(local_tag, "Heap after callback: %lu", esp_get_free_heap_size());
        return;
    }

    if (strcmp(action->valuestring, "length") == 0)
    {
        cJSON *length = cJSON_GetObjectItem(root, "length");
        if (length && cJSON_IsNumber(length))
        {
            data->expected = length->valueint;
            data->cur_index = 0;
            ESP_LOGI(local_tag, "Expecting %d tasks", data->expected);
        }
        cJSON_Delete(root);
        ESP_LOGW(local_tag, "Heap after callback: %lu", esp_get_free_heap_size());
        return;
    }

    if (strcmp(action->valuestring, "response") == 0)
    {
        ESP_LOGI(local_tag, "Server response index %d", data->cur_index);

        // Save json to file for later parsing
        esp_err_t rc = append_payload_to_file(payload);
        if (rc != ESP_OK)
        {
            ESP_LOGE(local_tag, "Failed to save reponse to disk!");
        }

        data->cur_index++;
        cJSON_Delete(root);
        ESP_LOGW(local_tag, "Heap after callback: %lu", esp_get_free_heap_size());
        return;
    }

    if (strcmp(action->valuestring, "ack") == 0)
    {
        ESP_LOGI(local_tag, "Server acknowledged update!");
        data->update_ack = 1;
        cJSON_Delete(root);
        ESP_LOGW(local_tag, "Heap after callback: %lu", esp_get_free_heap_size());
        return;
    }

    // Clean up by default if nothing matched
    cJSON_Delete(root);
    ESP_LOGW(local_tag, "Heap after callback: %lu", esp_get_free_heap_size());
}

#ifndef CONFIG_DEVICE_ID
#define CONFIG_DEVICE_ID "55"
#endif
#define RETRY_DELAY_MS 5000U

int request_backup(struct callback_data_t *cb_data)
{
    int return_status = EXIT_SUCCESS;
    size_t retries = 4;

    // Request payloads for all 3 entry types
    static const char *backup_payload[3] = {"{\"id\":\"" CONFIG_DEVICE_ID "\",\"action\":\"refresh\",\"type\":\"task\"}",
                                            "{\"id\":\"" CONFIG_DEVICE_ID "\",\"action\":\"refresh\",\"type\":\"event\"}",
                                            "{\"id\":\"" CONFIG_DEVICE_ID "\",\"action\":\"refresh\",\"type\":\"habit\"}"};

    for (int ent_itr = 0; ent_itr < 3; ent_itr++)
    {
        // heap_caps_monitor_local_minimum_free_size_start();

        return_status = mqtt_publish(backup_payload[ent_itr], strlen(backup_payload[ent_itr]));
        // ESP_LOGW(TAG, "Min heap during publish: %d bytes",
        // heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT));
        mqtt_loop(5000);

        /*
        while (1)
        {
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
        */
    }

    return return_status;
}

int sync_database(struct callback_data_t *cb_data)
{
    // heap_caps_monitor_local_minimum_free_size_start();

    if (!is_wifi_connected())
    {
        ESP_LOGW(TAG, "Unable to sync database: Device not connected to access point");
        return EXIT_FAILURE;
    }

    // Establish connection
    if (mqtt_connect() != EXIT_SUCCESS)
    {
        wifi_connected = false;
        ESP_LOGE(TAG, "MQTT connect failed. Aborting backup request.");
        // ESP_LOGW(TAG, "Min heap during connect: %d bytes",
        // heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT));
        return EXIT_FAILURE;
    }
    wifi_connected = true;
    // ESP_LOGW(TAG, "Min heap during connect: %d bytes",
    // heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT));

    if (mqtt_subscribe() != EXIT_SUCCESS)
    {
        ESP_LOGE(TAG, "MQTT subscribe failed. Disconnecting and aborting.");
        mqtt_disconnect();
        return EXIT_FAILURE;
    }
    // ESP_LOGW(TAG, "Min heap during subscribe: %d bytes",
    // heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT));

    // Send outgoing requests
    UploadTaskRequests(cb_data, CONFIG_DEVICE_ID);
    UploadHabitRequests(cb_data, CONFIG_DEVICE_ID);
    UploadEventRequests(cb_data, CONFIG_DEVICE_ID);

    // ESP_LOGW(TAG, "Min heap during request push: %d bytes",
    // heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT));

    // Populate database
    // ESP_LOGI(TAG, "Largest free block seen by request_backup: %d", //heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
    request_backup(cb_data);

    // Disconnect
    mqtt_unsubscribe();
    mqtt_disconnect();

    ESP_LOGI(TAG, "Parsing retrieved responses to database...");

    sqlite3_memory_highwater(1); // Reset highwater
    ParseJSONFileToDatabase(MESSAGE_BUFFER_NAME);
    ESP_LOGI(TAG, "Max SQLite memory used: %lld bytes", sqlite3_memory_highwater(0));

    return EXIT_SUCCESS;
}

void app_main()
{
    // heap_caps_monitor_local_minimum_free_size_start();
    esp_log_level_set("*", ESP_LOG_INFO);

    // Set up for database
    /*
    if (init_sqlite_memory() != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for SQLite! aborting...");
        return;
    }
    */

    // ------------------------------------- Set Up Wifi ------------------------------------------
    ESP_LOGW("main::Setting up Wifi", "Free heap total: %lu bytes", esp_get_free_heap_size());

    esp_log_level_set("httpd_uri", ESP_LOG_ERROR);
    esp_log_level_set("httpd_txrx", ESP_LOG_ERROR);
    esp_log_level_set("httpd_parse", ESP_LOG_ERROR);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());

        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    setup_wifi();

    // -------------------------------------- Initialize Storage ----------------------------------
    ESP_ERROR_CHECK(MountSDCard());

    // ------------------------------- Initialize Server Connection -------------------------------
    ESP_LOGW("main::Initialize Server Connection", "Free heap total: %lu bytes", esp_get_free_heap_size());

    struct callback_data_t cb_data;
    cb_data.expected = 0;
    cb_data.cur_index = -1;

    // Initialize mqtt library
    assert(mqtt_init(&callback, (void *)&cb_data) == EXIT_SUCCESS);

    // sync_database(&cb_data);

    // -------------------------------- Configure Clock (PCF8523) ---------------------------------
    ESP_LOGW("main::Configure Clock", "Free heap total: %lu bytes", esp_get_free_heap_size());

    // Set up timezone (global environment variable) (pst)
    setenv("TZ", "PST8PDT,M3.2.0/2,M11.1.0/2", 1);
    tzset();

    // Establish R2C connection
    ESP_ERROR_CHECK(InitRTC());
    ESP_ERROR_CHECK(RebootRTC()); // RECONFIGURE RTC configuration (optional)

    if (is_wifi_connected())
    {
        int return_status = mqtt_connect();
        if (return_status == EXIT_SUCCESS)
        {
            wifi_connected = true;

            int rc = SetTime();
            if (rc == ESP_OK)
            {
                RTC_updated = true;
            }
        }
        mqtt_disconnect();
    }
    else
    {
        ESP_LOGW(TAG, "Missing connection to server, skipping RTC sync...");
    }

    // ------------------------------------- Initialize LCD ---------------------------------------
    // ESP_LOGI("main::Initialize LCD", "Largest free block after clock init: %d", //heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
    ESP_LOGW("main::Initialize LCD", "Free heap total: %lu bytes", esp_get_free_heap_size());
    /* LCD HW initialization */
    ESP_ERROR_CHECK(app_lcd_init());

    /* LVGL initialization */
    ESP_ERROR_CHECK(app_lvgl_init());

    // --------------------------------- Create Task Under Load -----------------------------------
    // {
    //     ESP_LOGI(TAG, "Adding a Task Under Load of LCD...");

    //     // Define a task

    //     // current time
    //     time_t t = time(NULL);

    //     task_t newTask = {
    //         .uuid = "67890",
    //         .name = "Pet a dog",
    //         .description = "It's great for you",
    //         .completion = INCOMPLETE,
    //         .priority = 3,
    //         .time = t,
    //     };

    //     // Add task
    //     int rc = AddTaskDB(&newTask);
    //     if (rc != SQLITE_OK)
    //     {
    //         ESP_LOGE(TAG, "Failed to add task!");
    //         return;
    //     }
    //     ESP_LOGI(TAG, "Created Task!\n");
    // }

    // --------------------------------------- Runtime --------------------------------------------
    ESP_LOGW("main::Entering Runtime", "Free heap total: %lu bytes", esp_get_free_heap_size());

    app_main_display();

    long int frame_timer = 0;
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        lv_timer_handler(); // update screen
        frame_timer++;

        // Update time
        if (frame_timer % 10 == 0)
        {
            wifiDisplay(wifi_connected); // Displays if connected to broker

            struct tm currTime;
            pcf8523_read_time(&currTime);
            char timeBuffer[64];
            strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S %m-%d-%y %a", &currTime);
            timeDisplay(timeBuffer);
            updateFocusTimer();
        }

        // Request from server
        if (frame_timer >= 3000 && is_wifi_connected()) // ~30 seconds
        {
            loadMsgCreate();
            vTaskDelay(pdMS_TO_TICKS(10));
            // suspend lvgl
            lvgl_port_lock(0);

            ESP_LOGI(TAG, "Preforming Server Sync!");

            // Sync time if not done yet
            if (!RTC_updated && wifi_connected)
            {
                int rc = SetTime();
                if (rc == ESP_OK)
                {
                    RTC_updated = true;
                }
            }

            sync_database(&cb_data);

            frame_timer = 0;

            // resume lvgl
            lvgl_port_unlock();
            vTaskDelay(pdMS_TO_TICKS(10));
            //update draw content
            updateTaskBuff();
            drawTasks();
            updateEventBuff();
            drawEvents();
            updateHabitBuff();
            drawHabits();
            loadMsgRemove();
        }
    }
}

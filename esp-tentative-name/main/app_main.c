#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "mqtt_conn.h"

#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "pcf8523.h"

#include "sender.h"

#include "cJSON.h"

#include "esp_log.h"

static const char *TAG = "MAIN";

struct callback_data_t
{
    int expected;
    int cur_index;
    sqlite3 *db_ptr;
};

void demo_callback(const char *payload, size_t payload_length, void *cb_data)
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
            // Add task information to database
            ParseTasksJSON(data->db_ptr, payload);
            data->cur_index++;
        }
    }
    cJSON_Delete(root);
    ESP_LOGI(TAG, "Callback function called\n");
}

#define BACKUP_PAYLOAD "{\"id\":\"c72572d0-8c8c-4f37-8ff6-829cac2eabec\",\"action\":\"refresh\"}"
#define BACKUP_PAYLOAD_LENGTH ((size_t)(sizeof(BACKUP_PAYLOAD) - 1))
#define RETRY_DELAY_MS 5000U

int request_backup(struct callback_data_t *cb_data)
{
    int return_status = EXIT_SUCCESS;
    size_t retries = 4;
    mqtt_connect();
    mqtt_subscribe();

    mqtt_publish(BACKUP_PAYLOAD, BACKUP_PAYLOAD_LENGTH);
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
            mqtt_publish(BACKUP_PAYLOAD, BACKUP_PAYLOAD_LENGTH);
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

    mqtt_unsubscribe();
    mqtt_disconnect();
    return return_status;
}

// TODO
#if 0
#define TASK_LIST_SIZE 7

struct packed_t {
  struct task_t on_screen[TASK_LIST_SIZE],
  bool req_db_updates,
  bool push_new_completions_local,
  bool push_new_completions_mqtt
};
#endif

void app_main()
{
    esp_log_level_set("*", ESP_LOG_INFO);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());

        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(example_connect());

    struct callback_data_t cb_data;
    cb_data.expected = 0;
    cb_data.cur_index = -1;

    // Create new database object
    sqlite3 *db;
    InitSQL(&db);
    cb_data.db_ptr = db;

    // -------------------------------- Configure Clock (PCF8523) ---------------------------------
    /*if (!i2c_scan())
    {
        return;
    }*/
    ESP_ERROR_CHECK(InitRTC());
    ESP_ERROR_CHECK(RebootRTC());
    ESP_ERROR_CHECK(SetTime());

    // -------------------------------------- TEST SCRIPTS ----------------------------------------
    TestEventFunctions(db);

    // ------------------------------- Initialize Server Connection -------------------------------
    int return_status;
    return_status = mqtt_init(&demo_callback, (void *)&cb_data);
    assert(return_status == EXIT_SUCCESS);

    // TODO refill on_screen with the 0th page of tasks
    // retrievetaskssorted(&database, &(ui_data.on_screen), TASK_LIST_SIZE);

    request_backup(&cb_data);

#if 0
  long frame_timer = 0;
  while (1){
    // update the screen
    frame_timer++;

    // if there has been inputs where the user needs new tasks 
    // on the screen, modify the req_db_updates variables inside 
    // of the ui library
    if (req_db_updates) {
      // refill the on_screen array
      retrievetaskssorted(&db, &on_screen, TASK_LIST_SIZE, 0);
      req_db_updates = false;
    }

    // if tasks have been marked as completed this frame, modify
    // the push_new_completions variable inside the ui library
    if (push_new_completions_local) {
      // push changes to local db
      // push changes to disk
      push_new_completions_local = false;
      push_new_completions_mqtt = true;
      // possibly mark how many and/or which ones need to be pushed to the server
    }

    if (frame_timer >= 900 seconds) {
        if (push_new_completions_mqtt){
          // publish the "these tasks have been updates" payload
          push_new_completions_mqtt = false;
        }
        // publish the "need new tasks" payload
        request_backup(&cb_data);
        frame_timer = 0;
    }
  }
#endif

    // Close database
    CloseSQL(&db);
}

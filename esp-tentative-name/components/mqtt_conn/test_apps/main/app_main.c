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

#include "cJSON.h"

#include "esp_log.h"

static const char *TAG = "TEST_APP";

struct callback_data {
  int expected;
  int cur_index;
};

void demo_callback(const char *payload, size_t payload_length, void *cb_data) {
  struct callback_data *data = (struct callback_data *) cb_data;
  cJSON *root = cJSON_ParseWithLength(payload, payload_length);
  cJSON *item;
  item = cJSON_GetObjectItem(root, "id");
  if (item && (strcmp(item->valuestring, "server") == 0)){
    ESP_LOGI(TAG, "payload comes from server");
    item = cJSON_GetObjectItem(root, "action"); 
    if (item && (strcmp(item->valuestring, "length") == 0)) {
      item = cJSON_GetObjectItem(root, "length");
      if (item && cJSON_IsNumber(item)) {
        data->expected = item->valueint;
        data->cur_index = 0;
        ESP_LOGI(TAG, "Expecting %d tasks", data->expected);
      }
    }
    else if (item && (strcmp(item->valuestring, "response") == 0)){
      ESP_LOGI(TAG, "Server response index %d", data->cur_index);
      data->cur_index ++;
    }
  }
  cJSON_Delete(root);
  ESP_LOGI(TAG, "Callback function called\n");
}

void app_main() {
  esp_log_level_set("*", ESP_LOG_INFO);

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());

    ESP_ERROR_CHECK(nvs_flash_init());
  }

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());


  char payload[] = "{\"id\":\"55\",\"action\":\"refresh\",\"type\":\"event\"}";
  size_t payload_length = sizeof(payload) - 1;

  struct callback_data cb_data;
  cb_data.expected = 0;
  cb_data.cur_index = -1;

  int return_status;
  return_status = mqtt_init(&demo_callback, (void *) &cb_data);

  setup_wifi();

  return_status = mqtt_connect();
  while (return_status != EXIT_SUCCESS){
    ESP_LOGI(TAG, "Not able to connect");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    return_status = mqtt_connect();
  }
  mqtt_subscribe();

  mqtt_publish(payload, payload_length);
  while (1) {
    mqtt_loop(5000);
    if (cb_data.expected == 0) {
      ESP_LOGI(TAG, "Did not get back a length message from the server, re-publishing after a delay");
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      mqtt_publish(payload, payload_length);
    }
    else if (cb_data.expected != cb_data.cur_index){
      ESP_LOGI(TAG, "Did not get the expected amount, listening for some more time");
    }
    else{
      break;
    }
  }

  mqtt_unsubscribe();
  mqtt_disconnect();

  ESP_LOGI(TAG, "Exit status %d", return_status);
}

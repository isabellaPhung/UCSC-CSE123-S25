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

#define CONFIG_DEVICE_ID "c72572d0-8c8c-4f37-8ff6-829cac2eabec"

static const char *TAG = "TEST_APP";

struct callback_data {
  void *placeholder;
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
      }
    }
    else if (item && (strcmp(item->valuestring, "response") == 0)){
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


  char payload[] = "{\"id\":\"" CONFIG_DEVICE_ID "\",\"action\":\"refresh\",\"type\":\"event\"}";
  size_t payload_length = sizeof(payload) - 1;

  ESP_LOGI(TAG, "Testeing device ID: %s", CONFIG_DEVICE_ID);

  struct callback_data cb_data;

  int return_status;
  return_status = mqtt_init(&demo_callback, (void *) &cb_data);

  setup_wifi();

  while (!is_wifi_connected()){
    ESP_LOGI(TAG, "Wifi disconnected");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }

  return_status = mqtt_connect();

  while (return_status != EXIT_SUCCESS){
    ESP_LOGI(TAG, "Not able to connect");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    return_status = mqtt_connect();
  }
  mqtt_subscribe();

  // do once after booting
  mqtt_publish(payload, payload_length);
  mqtt_loop(5000);

  int seconds = 30;
  if (seconds >= 30) {
    // publish update as needed
    mqtt_loop(5000);
  }

  // when shutting down
  mqtt_unsubscribe();
  mqtt_disconnect();

  ESP_LOGI(TAG, "Exit status %d", return_status);
}

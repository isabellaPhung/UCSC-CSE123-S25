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

#include "esp_log.h"

static const char *TAG = "MQTT_EXAMPLE";

void demo_callback(const char *payload, size_t payload_length) {
  (void) payload;
  (void) payload_length;
  ESP_LOGI(TAG, "Callback function called\n");
  return;
}

void app_main() {
  ESP_LOGI(TAG, "[APP] Startup..");
  ESP_LOGI(TAG, "[APP] Free memory: %"PRIu32" bytes", esp_get_free_heap_size());
  ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

  esp_log_level_set("*", ESP_LOG_INFO);

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());

    ESP_ERROR_CHECK(nvs_flash_init());
  }

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  ESP_ERROR_CHECK(example_connect());

  char payload[] = "{\"id\":\"c72572d0-8c8c-4f37-8ff6-829cac2eabec\",\"action\":\"refresh\"}";
  size_t payload_length = sizeof(payload) - 1;

  int return_status;
  return_status = init_mqtt(&demo_callback);
  if (return_status != EXIT_SUCCESS) {
    ESP_LOGE(TAG, "Failed initializing mqtt");
    return;
  }
  do {
    return_status = publish_packet(payload, payload_length);
  } while (return_status != EXIT_SUCCESS);
  ESP_LOGI(TAG, "Exit status %d", return_status);
}

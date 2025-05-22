idf_component_register(
  SRCS
    "app_main.c"
  REQUIRES
    "mqtt_conn"
    "nvs_flash"
    "esp_event"
    "esp_netif"
    "json"
    "wifi_setup"
  INCLUDE_DIRS "."
)

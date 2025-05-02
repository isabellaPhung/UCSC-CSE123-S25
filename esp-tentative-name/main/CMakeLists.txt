idf_component_register(
  SRCS
    "app_main.c"
    "intermediary.c"
  REQUIRES
    "mqtt_conn"
    "UI_ili9488"
    "nvs_flash"
    "esp_event"
    "esp_netif"
    "protocol_examples_common"
    "json"
    "database"
  INCLUDE_DIRS "."
)

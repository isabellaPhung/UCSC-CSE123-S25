idf_component_register(
  SRCS
    "app_main.c"
  REQUIRES
    "mqtt_conn"
    "ili9488"
    "nvs_flash"
    "esp_event"
    "esp_netif"
    "protocol_examples_common"
  INCLUDE_DIRS "."
)

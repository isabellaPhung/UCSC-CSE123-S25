idf_component_register(
  SRCS
    "app_main.c"
    "sender.c"
  REQUIRES
    "mqtt_conn"
    "UI_ili9488"
    "pcf8523"
    "esp_lcd"
    "lvgl__lvgl"
    "espressif__esp_lvgl_port"
    "atanisoft__esp_lcd_ili9488"
    "espressif__button"
    "database"
    "nvs_flash"
    "esp_event"
    "esp_netif"
    "protocol_examples_common"
    "json"

  INCLUDE_DIRS "."
)

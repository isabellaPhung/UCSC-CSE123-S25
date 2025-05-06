idf_component_register(
  SRCS
    "main.c"
  REQUIRES
    "UI_ili9488"
    "esp_lcd"
    "lvgl__lvgl"
    "espressif__esp_lvgl_port"
    "atanisoft__esp_lcd_ili9488"
    "espressif__button"
    "database"
    "json"
  INCLUDE_DIRS "."
)

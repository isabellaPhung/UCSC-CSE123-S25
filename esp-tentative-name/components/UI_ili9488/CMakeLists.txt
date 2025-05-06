file(GLOB_RECURSE SOURCES "src/*.c")

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "include"
    REQUIRES
        "esp_lcd"
        "lvgl__lvgl"
        "espressif__esp_lvgl_port"
        "atanisoft__esp_lcd_ili9488"
        "espressif__button"
        "database"
)

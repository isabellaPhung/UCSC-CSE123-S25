idf_component_register(
    SRCS "main.c"
    REQUIRES
        "esp_lcd"
        "lvgl"
        "espressif__esp_lvgl_port"
        "atanisoft__esp_lcd_ili9488"
        "espressif__button"
        "UI_ili9488"
        "database"
        INCLUDE_DIRS "../../include")
target_compile_options(${COMPONENT_LIB} PRIVATE  -Wno-error=implicit-function-declaration)

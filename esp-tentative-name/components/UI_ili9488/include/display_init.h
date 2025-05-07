#pragma once

#include "definitions.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lvgl_port.h" //ports lvgl to esp
#include <esp_lcd_ili9488.h> //screen driver
#include "ui.h"

//initializes LCD
esp_err_t app_lcd_init(void);

//initializes LVGL
esp_err_t app_lvgl_init(void);

//draws UI
void app_main_display();

/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "display_init.h"

void app_main(void)
{
    /* LCD HW initialization */
    ESP_ERROR_CHECK(app_lcd_init());

    /* LVGL initialization */
    ESP_ERROR_CHECK(app_lvgl_init());

    /* All the GUI drawing */
    app_main_display();
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));   // I can't remember why I put this delay here
        lv_timer_handler();              // update screen
        vTaskDelay(pdMS_TO_TICKS(5000)); // delay
    }
}

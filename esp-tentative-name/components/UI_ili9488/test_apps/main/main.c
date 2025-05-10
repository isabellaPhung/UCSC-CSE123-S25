/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "display_init.h"
//#include "esp_log.h"
//static const char *HEAP = "Heap Check";

void app_main(void)
{
    //roughly checking heap usage before init LVGL
    //ESP_LOGI(HEAP, "Largest free block before LVGL init: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
    
    /* LCD HW initialization */
    ESP_ERROR_CHECK(app_lcd_init());

    /* LVGL initialization */
    ESP_ERROR_CHECK(app_lvgl_init());

    /* All the GUI drawing */
    app_main_display();
    
    //roughly checking heap usage after drawing GUI
    //ESP_LOGI(HEAP, "largest free block after LVGL: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(5000)); // delay
        lv_timer_handler();              // update screen
    }
}

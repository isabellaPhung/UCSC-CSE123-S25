/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "database.h" //for database purposes
#include "display_init.h"  //initalizes LVGL and display hardware

void initDatabase(){

}

void adjustDatabase(){

}

void app_main(void){
    initDatabase();
    /* All the GUI stuff */
    app_main_display();
    adjustDatabase();
    while(1){
        vTaskDelay(pdMS_TO_TICKS(10)); //I can't remember why I put this delay here
        lv_timer_handler(); //update screen
        vTaskDelay(pdMS_TO_TICKS(5000)); //delay
    }
}

/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "database.h" //for database purposes
#include "display_init.h"  //initalizes LVGL and display hardware
#include "pcf8523.h" //for RTC
#include "helper_menus.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
//#include "esp_netif.h"
//#include "esp_event.h"
/*
create_task(tasklist, "Capstone Project", "3/25/2025");
create_task(tasklist, "Figure out Prototype", "3/29/2025");
create_task(tasklist, "Learn PCB Design", "3/30/2025");
create_task(tasklist, "Learn Computer Aided Design", "4/1/2025");
*/

static const char *TAG = "SQL_DEMO";
static const char *HEPLE = "HEPLE";
static const char *RTCTAG = "RTCTAG";
static const char *NETTAG = "NETTAG";

sqlite3 * db;
void initdb(){
    ESP_LOGI(TAG, "SQL Database Tests For Schedule Companion\n");

    // ---------------------------------- Create Databse ----------------------------------
    ESP_LOGI(TAG, "Initializing database...");
    // Initialize the database and table
    if (InitSQL(&db) != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize database!");
        return;
    }
    ESP_LOGI(TAG, "Database initialized!\n");
    
    // ----------------------------------- Create Task ------------------------------------
    ESP_LOGI(TAG, "Adding a Task...");
    // Define a task

    // current time
    time_t t = time(NULL);
    
    task_t newTask = {
        .uuid = "12345",
        .name = "Capstone Project",
        .description = "Complete Capstone Project",
        .completion = INCOMPLETE,
        .priority = 5,
        .time = t,
    };

    // Add task
    int rc = AddTaskDB(db, &newTask);
    if (rc != SQLITE_OK){
        ESP_LOGE(TAG, "Failed to add task!");
        return;
    }
    ESP_LOGI(TAG, "Created Task!\n");
    
    /*
    //trying to get the task info in the GUI
    struct tm* ptr;
    ptr = localtime(&(newTask.time));
    */
}

void adjustDatabase(){

}
void app_main(void){
    ESP_LOGI(HEPLE, "start heap: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
    
    /* Database initialization */
    initdb();
    ESP_LOGI(HEPLE, "largest free block after database init: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));

    /* LCD HW initialization */
    ESP_ERROR_CHECK(app_lcd_init());

    /* LVGL initialization */
    ESP_ERROR_CHECK(app_lvgl_init());

    initDatabase(db);
    /* All the GUI drawing */
    app_main_display();
    ESP_LOGI(HEPLE, "largest free block after LVGL: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
    //heap_caps_print_heap_info(MALLOC_CAP_DEFAULT); //heap info
    
    /* 
    // RTC initialization
    if (!i2c_scan()){
        ESP_LOGE(RTCTAG, "No I2C devices found!");
        return;
    }
    ESP_ERROR_CHECK(InitRTC());
    ESP_ERROR_CHECK(RebootRTC());
    //set time, but needs internet connection. tried to get it working but it wouldn't cooperate.
    ESP_LOGI(HEPLE, "largest free block after RTC init: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
    
    time_t rtc_time;
    struct tm *tm_info = localtime(&rtc_time);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%D %r", tm_info);
    pcf8523_read_time(&rtc_time);
    timeDisplay(buffer);
    */
    //adjustDatabase();
    while(1){
        vTaskDelay(pdMS_TO_TICKS(10)); 
        //pcf8523_read_time(&rtc_time);
        //timeDisplay(buffer); //update time on 
        lv_timer_handler(); //update screen
    }
    //CloseSQL(&db);
}

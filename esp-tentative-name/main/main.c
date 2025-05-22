/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "database.h" //for database purposes
#include "display_init.h"  //initalizes LVGL and display hardware
#include "helper_menus.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

#include "pcf8523.h" //for RTC
/*
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "wifi_setup.h"
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
        .uuid = "help",
        .name = "Capstone Project",
        .description = "Complete Capstone Project",
        .completion = INCOMPLETE,
        .priority = 5,
        .time = t,
    };

    // Add task
    AddTaskDB(&newTask);
    
    ESP_LOGI(TAG, "Created Task!\n");
    task_t newTask1 = {
        .uuid = "help1",
        .name = "Capstone Project1",
        .description = "Complete Capstone Project1",
        .completion = INCOMPLETE,
        .priority = 5,
        .time = t,
    };

    // Add task
    AddTaskDB(&newTask1);
    
    task_t newTask2 = {
        .uuid = "help2",
        .name = "Capstone Project2",
        .description = "Complete Capstone Project2",
        .completion = INCOMPLETE,
        .priority = 5,
        .time = t,
    };

    // Add task
    AddTaskDB(&newTask2);
    
    task_t newTask3 = {
        .uuid = "help3",
        .name = "Capstone Project3",
        .description = "Complete Capstone Project3",
        .completion = INCOMPLETE,
        .priority = 5,
        .time = t,
    };

    // Add task
    AddTaskDB(&newTask3);
    
    task_t newTask4 = {
        .uuid = "help4",
        .name = "Capstone Project4",
        .description = "Complete Capstone Project4",
        .completion = INCOMPLETE,
        .priority = 5,
        .time = t,
    };
    // Add task
    AddTaskDB(&newTask4);

    task_t newTask5 = {
        .uuid = "help5",
        .name = "Capstone Project5",
        .description = "Complete Capstone Project5",
        .completion = INCOMPLETE,
        .priority = 5,
        .time = t,
    };
    // Add task
    AddTaskDB(&newTask5);

    task_t newTask6 = {
        .uuid = "help6",
        .name = "Capstone Project6",
        .description = "Complete Capstone Project6",
        .completion = INCOMPLETE,
        .priority = 5,
        .time = t,
    };
    
    // Add task
    AddTaskDB(&newTask6);
    
    task_t newTask7 = {
        .uuid = "help7",
        .name = "Capstone Project7",
        .description = "Complete Capstone Project7",
        .completion = INCOMPLETE,
        .priority = 5,
        .time = t,
    };
    
    // Add task
    AddTaskDB(&newTask7);
    
    task_t newTask8 = {
        .uuid = "help8",
        .name = "Capstone Project8",
        .description = "Complete Capstone Project8",
        .completion = INCOMPLETE,
        .priority = 5,
        .time = t,
    };
    
    // Add task
    AddTaskDB(&newTask8);

    // ----------------------------------- Create Event ------------------------------------
   /* 
    ESP_LOGI(TAG, "Adding an Event...");
    // Define a task

    event_t newEvent = {
        .uuid = "event0",
        .name = "Meet with Capstone Team0",
        .description = "Capstone Meeting0",
        .start_time = t,
        .duration = 7200,
    };

    // Add event
    AddEventDB(&newEvent);
    
    ESP_LOGI(TAG, "Created Event!\n");
    
    ESP_LOGI(TAG, "Adding an Event...");
    // Define a task

    event_t newEvent1 = {
        .uuid = "event1",
        .name = "Meet with Capstone Team1",
        .description = "Capstone Meeting1",
        .start_time = t,
        .duration = 7200,
    };

    // Add event
    AddEventDB(&newEvent1);
   
    ESP_LOGI(TAG, "Created Event!\n");
    
    ESP_LOGI(TAG, "Adding an Event...");
    // Define a task

    event_t newEvent2 = {
        .uuid = "event2",
        .name = "Meet with Capstone Team2",
        .description = "Capstone Meeting2",
        .start_time = t,
        .duration = 7200,
    };

    // Add event
    AddEventDB(&newEvent2);
   
    ESP_LOGI(TAG, "Created Event!\n");
    
    ESP_LOGI(TAG, "Adding an Event...");
    // Define a task

    event_t newEvent3 = {
        .uuid = "event3",
        .name = "Meet with Capstone Team3",
        .description = "Capstone Meeting3",
        .start_time = t,
        .duration = 7200,
    };

    // Add event
    AddEventDB(&newEvent3);
    
    ESP_LOGI(TAG, "Created Event!\n");

    ESP_LOGI(TAG, "Adding an Event...");
    // Define a task

    event_t newEvent4 = {
        .uuid = "event4",
        .name = "Meet with Capstone Team4",
        .description = "Capstone Meeting4",
        .start_time = t,
        .duration = 7200,
    };

    // Add event
    AddEventDB(&newEvent4);
        ESP_LOGI(TAG, "Created Event!\n");

    ESP_LOGI(TAG, "Adding an Event...");
    // Define a task

    event_t newEvent5 = {
        .uuid = "event5",
        .name = "Meet with Capstone Team5",
        .description = "Capstone Meeting5",
        .start_time = t,
        .duration = 7200,
    };

    // Add event
    AddEventDB(&newEvent5);
    
    ESP_LOGI(TAG, "Created Event!\n");
    
    ESP_LOGI(TAG, "Adding an Event...");
    // Define a task

    event_t newEvent6 = {
        .uuid = "event6",
        .name = "Meet with Capstone Team6",
        .description = "Capstone Meeting6",
        .start_time = t,
        .duration = 7200,
    };

    // Add event
    AddEventDB(&newEvent6);
    
    ESP_LOGI(TAG, "Created Event!\n");

    ESP_LOGI(TAG, "Adding an Event...");
    // Define a task

    event_t newEvent7 = {
        .uuid = "event7",
        .name = "Meet with Capstone Team7",
        .description = "Capstone Meeting7",
        .start_time = t,
        .duration = 7200,
    };

    // Add event
    AddEventDB(&newEvent7);
    
    ESP_LOGI(TAG, "Created Event!\n");

    ESP_LOGI(TAG, "Adding an Event...");
    // Define a task

    event_t newEvent8 = {
        .uuid = "event8",
        .name = "Meet with Capstone Team8",
        .description = "Capstone Meeting8",
        .start_time = t,
        .duration = 7200,
    };

    // Add event
    AddEventDB(&newEvent8);
    
    ESP_LOGI(TAG, "Created Event!\n");
    */

    // ----------------------------------- Create Habit ------------------------------------
    ESP_LOGI(TAG, "Adding a Habit...");
    // Define a habit
    
    HabitAddDB("23456", "Walk the Dog", 8);
    
    ESP_LOGI(TAG, "Created Habit!\n");
    
    // ----------------------------------- Create Habit ------------------------------------
    ESP_LOGI(TAG, "Adding a Habit...");
    // Define a habit

    HabitAddDB("habhab", "Study Electronics", 1);
    
    ESP_LOGI(TAG, "Created Habit!\n");
    
    // ----------------------------------- Create Habit ------------------------------------
    ESP_LOGI(TAG, "Adding a Habit...");
    // Define a habit

    HabitAddDB("habby", "Eat vegetables", 8);
    
    ESP_LOGI(TAG, "Created Habit!\n");
    
    // ----------------------------------- Create Habit ------------------------------------
    ESP_LOGI(TAG, "Adding a Habit...");
    // Define a habit

    HabitAddDB("hub", "Practice LEETcode", 8);
    
    ESP_LOGI(TAG, "Created Habit!\n");

}

void adjustDatabase(){
// ----------------------------------- Create 2nd Task ------------------------------------
    ESP_LOGI(TAG, "Adding a 2nd Task...");
    // Define a task

    // current time
    time_t t = time(NULL);
    
    task_t newTask = {
        .uuid = "777",
        .name = "Finish ECE173 Homework",
        .description = "Design PCB for ground bounce",
        .completion = INCOMPLETE,
        .priority = 2,
        .time = t,
    };

    // Add task
    AddTaskDB(&newTask);
    
    ESP_LOGI(TAG, "Created 2nd Task!\n");

    ESP_LOGI(TAG, "Adding an Event...");
    // Define a task

    event_t newEvent = {
        .uuid = "toes",
        .name = "Meet with Capstone Team",
        .description = "Meeting with Capstone Team",
        .start_time = t,
        .duration = 7200,
    };

    // Add event
    AddEventDB(&newEvent);
    
    ESP_LOGI(TAG, "Created Event!\n");

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

    //initDatabase(db);
    /* All the GUI drawing */
    app_main_display();
    ESP_LOGI(HEPLE, "largest free block after LVGL: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
    //heap_caps_print_heap_info(MALLOC_CAP_DEFAULT); //heap info
   
    /*
    //wifi setup
    esp_log_level_set("*", ESP_LOG_INFO);

    esp_log_level_set("httpd_uri", ESP_LOG_ERROR);
    esp_log_level_set("httpd_txrx", ESP_LOG_ERROR);
    esp_log_level_set("httpd_parse", ESP_LOG_ERROR);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(nvs_flash_init());

    setup_wifi();
    */
    
    // RTC initialization
    if (!i2c_scan()){
        ESP_LOGE(RTCTAG, "No I2C devices found!");
        return;
    }
    ESP_ERROR_CHECK(InitRTC());
    ESP_ERROR_CHECK(RebootRTC());
    //SetTime();
    ESP_LOGI(HEPLE, "largest free block after RTC init: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
    
    time_t rtc_time;
    pcf8523_read_time(&rtc_time);
    
    struct tm *tm_info = localtime(&rtc_time);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%H:%M:%S %m-%d-%y %a", tm_info);
    timeDisplay(buffer);
    //adjustDatabase();
    //wifiDisplay(false);
    //timerInit();
    //loadMsgCreate();
    while(1){
        vTaskDelay(pdMS_TO_TICKS(1000)); 
        //loadMsgRemove();
        //wifiDisplay(true);
        //readTimer();
        pcf8523_read_time(&rtc_time);
        tm_info = localtime(&rtc_time);
        strftime(buffer, sizeof(buffer), "%H:%M:%S %m-%d-%y %a", tm_info);
        timeDisplay(buffer); //update time
        updateFocusTimer();
        //if updated task, event or habit recieved, call corresponding function to update:
        //updateTaskBuff();
        //drawTasks();

        lv_timer_handler(); //update screen
    }
    //CloseSQL(&db);
}

/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "database.h" //for database purposes
#include "display_init.h"  //initalizes LVGL and display hardware
#include <time.h>
/*
create_task(tasklist, "Capstone Project", "3/25/2025");
create_task(tasklist, "Figure out Prototype", "3/29/2025");
create_task(tasklist, "Learn PCB Design", "3/30/2025");
create_task(tasklist, "Learn Computer Aided Design", "4/1/2025");
*/

void initDatabase(){
    static const char *TAG = "SQL_DEMO";

    sqlite3 *db;

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
        .uuid = "0",
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

    //trying to get the task info in the GUI
    struct tm* ptr;
    ptr = localtime(&(newTask.time));

    create_task(newTask.name, asctime(ptr));
}

void adjustDatabase(){

}

void app_main(void){
    /* All the GUI stuff */
    app_main_display();
    initDatabase();
    adjustDatabase();
    while(1){
        vTaskDelay(pdMS_TO_TICKS(10)); //I can't remember why I put this delay here
        lv_timer_handler(); //update screen
        vTaskDelay(pdMS_TO_TICKS(5000)); //delay
    }
    //CloseSQL(&db);
}

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include "lvgl__lvgl/lvgl.h"
//#include "esp_timer.h"
//#include "driver/timer.h"

#include "definitions.h"
#include "messenger.h"
#include "database.h"

void timerInit();
void readTimer();

//initializes necessary fonts
void initFonts();

//initializes groups
void initGroup();

//loads loading msg 
void loadMsgCreate(void);
//removes loading msg 
void loadMsgRemove(void);
//loads wifi screen
void loadWifiTile();
//loads task event menu
void loadTile2();
//loads habit menu
void loadTile3();

void loadPrevTasks();
void loadNextTasks();
void loadPrevEvents();
void loadNextEvents();

//updates time with given string
void timeDisplay(char * entry);

//displays wifi symbol with given boolean
void wifiDisplay(bool hasWifi);

//adds a task to the task event menu
void create_task(task_t * task);
//adds a event to the task event menu
void create_event(event_t * event);
//adds a habit to the habit menu
void createHabit(habit_t * habit);

//update functions, call according one after update recieved
void updateTaskBuff();
void drawTasks();
void updateEventBuff();
void drawEvents();
void updateHabitBuff();
void drawHabits();

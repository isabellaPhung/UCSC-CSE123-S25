#pragma once

#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include "lvgl__lvgl/lvgl.h"

#include "definitions.h"
#include "database.h"
#include "task.h"
#include "event.h"
#include "habit.h"

//initializes necessary fonts
void initFonts();

//initializes groups
void initGroup();

//initializes buffers
//void initBuffers();

//loads focus menu
void loadTile1();
//loads task event menu
void loadTile2();
//loads habit menu
void loadTile3();

//TODO?
void loadPrevTasks();
void loadNextTasks();
void loadPrevEvents();
void loadNextEvents();

//updates time with given string
void timeDisplay(char * entry);

//adds a task to the task event menu
void create_task(task_t * task);
//adds a event to the task event menu
void create_event(event_t * event);
//adds a habit to the habit menu
void createHabit(habit_t * habit);
void initTaskBuff();
void initEventBuff();
void initHabitBuff();




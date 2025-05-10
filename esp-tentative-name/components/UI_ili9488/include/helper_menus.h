#pragma once

#include <stddef.h>
#include <stdint.h>
#include "lvgl__lvgl/lvgl.h"

#include "definitions.h"

//initializes necessary fonts
void initFonts();

//initializes groups
void initGroup();

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
void create_task(const char * name, const char * dueDate);
//TODO
//adds a event to the task event menu
//void create_event(const char * name, const char * dueDate);
//adds a habit to the habit menu
//TODO fix the row stuff?
void createHabit(const char * name, uint8_t row);



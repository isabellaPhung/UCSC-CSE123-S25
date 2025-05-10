#pragma once
#include "definitions.h"
#include <stddef.h>
#include <stdint.h>
#include "lvgl__lvgl/lvgl.h"

void initFonts();
void initGroup();
void focusMenu_create(lv_obj_t * parent);
void create_task(const char * name, const char * dueDate);
void loadPrevTasks();
void loadNextTasks();
void loadPrevEvents();
void loadNextEvents();

//takes a static string and sets time
void timeDisplay();

void taskEvent_create(lv_obj_t * parent);
void createHabit(lv_obj_t * parent, const char * name, uint8_t row);
void habitMenu_create(lv_obj_t * parent);

void loadTile1();
void loadTile2();
void loadTile3();

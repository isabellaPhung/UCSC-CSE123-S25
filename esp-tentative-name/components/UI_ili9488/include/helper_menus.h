#pragma once
#include "definitions.h"
#include <stddef.h>
#include <stdint.h>
#include "lvgl__lvgl/lvgl.h"

static void focus_cb(lv_event_t * e);
static void taskevent_cb(lv_event_t * e);
static void habit_cb(lv_event_t * e);
void initFonts();
void initGroup();
void focusMenu_create(lv_obj_t * parent);
static void msgbox_cb(lv_event_t * e);
static void task_desc_cb(lv_event_t * e);
void create_task(const char * name, const char * dueDate);
void loadPrevTasks();
void loadNextTasks();
void loadPrevEvents();
void loadNextEvents();
static void tasks_left_cb(lv_event_t * e);
static void tasks_right_cb(lv_event_t * e);
static void events_left_cb(lv_event_t * e);
static void events_right_cb(lv_event_t * e);

static void button_nav_cb(lv_event_t * e);

void taskEvent_create(lv_obj_t * parent);
static void buttonmatrix_cb(lv_event_t * e);
void createHabit(lv_obj_t * parent, const char * name, uint8_t row);
void habitMenu_create(lv_obj_t * parent);

void loadTile1();
void loadTile2();
void loadTile3();

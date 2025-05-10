#pragma once

#include "esp_err.h"
#include "esp_log.h"
#include "lvgl__lvgl/lvgl.h"
#include "iot_button.h"
#include "button_adc.h"

#include "definitions.h"
#include "helper_menus.h" //Menu drawing functions

//initializes adc hardware buttons, and draws first menu of GUI
void create_ui();

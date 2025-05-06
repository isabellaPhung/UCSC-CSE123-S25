/* Button Pins */
#define BUTTON_PIN  1

#pragma once

#include "esp_err.h"
#include "esp_log.h"
#include "lvgl__lvgl/lvgl.h"
#include "iot_button.h"
#include "button_adc.h"
#include "helper_menus.h" //might need to make a header file in the future
                          //at the moment just call the c function directly

//initializes adc hardware buttons, and draws first menu of GUI
void create_ui();

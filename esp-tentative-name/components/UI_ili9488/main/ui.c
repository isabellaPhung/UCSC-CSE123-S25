//uses code from io_button repo's test code for GPIO buttons

#include "lvgl__lvgl/lvgl.h"

//assorted graphic items used across functions
static lv_obj_t * tile1;
static lv_obj_t * tile2;
static lv_obj_t * tile3;
static lv_group_t * g;
static lv_indev_t * keypad;
static button_handle_t btns[5] = {NULL};

#include "helper_menus.c"
#include "iot_button.h"
#include "button_adc.h"

static const char *BUTTON = "button";

void makeButtons(){
    // create ADC button
    const button_config_t btn_cfg = {0};
    button_adc_config_t btn_adc_cfg = {
        .adc_channel = 1,
    };
    size_t btnCount = 5;
    const uint16_t vol[5] = {300, 595, 1055, 1650, 2268};
    for(size_t i = 0; i < btnCount; i++){
        btn_adc_cfg.button_index = i;
         if (i == 0) {
            btn_adc_cfg.min = (0 + vol[i]) / 2;
        } else {
            btn_adc_cfg.min = (vol[i - 1] + vol[i]) / 2;
        }

        if (i == btnCount-1) {
            btn_adc_cfg.max = (vol[i] + 3000) / 2;
        } else {
            btn_adc_cfg.max = (vol[i] + vol[i + 1]) / 2;
        }
        esp_err_t ret = iot_button_new_adc_device(&btn_cfg, &btn_adc_cfg, &btns[i]);
        if(ret != ESP_OK) {
            ESP_LOGE(BUTTON, "Button create failed");
        }
    }
}


uint32_t last_key(){
    if(iot_button_get_event(btns[4]) == BUTTON_PRESS_DOWN){ //if up pressed
        return  LV_KEY_DOWN;
    }else if(iot_button_get_event(btns[3]) == BUTTON_PRESS_DOWN){ //if down pressed
        return  LV_KEY_UP;
    }else if(iot_button_get_event(btns[2]) == BUTTON_PRESS_DOWN){ //if left pressed
        return  LV_KEY_LEFT;
    }else if(iot_button_get_event(btns[1]) == BUTTON_PRESS_DOWN){ //if right pressed
        return  LV_KEY_RIGHT;
    }else if(iot_button_get_event(btns[0]) == BUTTON_PRESS_DOWN){ //if select pressed
        return  LV_KEY_ENTER;
    }
    return 0;
}

void keyboard_read(lv_indev_t * indev, lv_indev_data_t * data){
    data->key = last_key();
    if(data->key > 0){
        data->state = LV_INDEV_STATE_PRESSED;
    }else{
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

void keypad_init(){
    keypad = lv_indev_create();
    lv_indev_set_read_cb(keypad, keyboard_read);
    lv_indev_set_type(keypad, LV_INDEV_TYPE_KEYPAD);
}

void create_ui(){
    makeButtons();
    initFonts();
    keypad_init();
    g = lv_group_create();
    lv_group_set_default(g);
    lv_indev_set_group(keypad, g);

    loadTile1();
}

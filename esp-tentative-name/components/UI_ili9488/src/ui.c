#include "ui.h"

//assorted graphic items used across functions
static lv_indev_t * keypad;
static button_handle_t btns[5] = {NULL};

//for edge detecting
static button_event_t prev4;
static button_event_t prev2;
static button_event_t prev3;
static button_event_t prev1;
static button_event_t prev0;

static const char *BUTTON = "button"; //for esp_log

/// @brief initializes adc hardware buttons, and draws first menu of GUI
void makeButtons(){
    // create ADC button
    const button_config_t btn_cfg = {0};
    button_adc_config_t btn_adc_cfg = {
        .adc_channel = BUTTON_PIN,
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

/// @brief returns keyboard key
/// @return keyboard value for LVGL
uint32_t last_key(){
    //ESP_LOGI(BUTTON, "%s", iot_button_get_event_str(iot_button_get_event(btns[0])));
    uint32_t key = 0;
    if(prev4 == BUTTON_NONE_PRESS && iot_button_get_event(btns[4]) == BUTTON_PRESS_DOWN){ //if up pressed
        ESP_LOGI(BUTTON, "UP");
        key = LV_KEY_UP;
    }else if(prev3 == BUTTON_NONE_PRESS && iot_button_get_event(btns[3]) == BUTTON_PRESS_DOWN){ //if down pressed
        ESP_LOGI(BUTTON, "DOWN");
        key = LV_KEY_DOWN;
    }else if(prev2 == BUTTON_NONE_PRESS && iot_button_get_event(btns[2]) == BUTTON_PRESS_DOWN){ //if right pressed
        ESP_LOGI(BUTTON, "RIGHT");
        key = LV_KEY_RIGHT;
    }else if(prev1 == BUTTON_NONE_PRESS && iot_button_get_event(btns[1]) == BUTTON_PRESS_DOWN){ //if left pressed
        ESP_LOGI(BUTTON, "LEFT");
        key = LV_KEY_LEFT;
    }else if(prev0 == BUTTON_NONE_PRESS && iot_button_get_event(btns[0]) == BUTTON_PRESS_DOWN){ //if select pressed
        ESP_LOGI(BUTTON, "ENTER");
        key = LV_KEY_ENTER;
    }
    //for edge detection purposes
    prev4 = iot_button_get_event(btns[4]);   
    prev3 = iot_button_get_event(btns[3]);   
    prev2 = iot_button_get_event(btns[2]);   
    prev1 = iot_button_get_event(btns[1]);   
    prev0 = iot_button_get_event(btns[0]);   
    return key;
}

/// @brief keypad callback function 
/// @param indev lvgl input device 
/// @param data indev data 
void keyboard_read(lv_indev_t * indev, lv_indev_data_t * data){
    data->key = last_key();
    if(data->key > 0){
        data->state = LV_INDEV_STATE_PRESSED;
    }else{
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

/// @brief initializes buttons as a lvgl keypad
void keypad_init(){
    keypad = lv_indev_create();
    lv_indev_set_read_cb(keypad, keyboard_read);
    lv_indev_set_type(keypad, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_group(keypad, lv_group_get_default());
}


/// @brief inits necessary fonts and buttons and draws ui
void create_ui(){
    makeButtons();
    initFonts();
    initGroup();
    //initBuffers();
    keypad_init();

    tasks_right_cb();
    events_right_cb();
    habits_right_cb();
    loadTile1();
}

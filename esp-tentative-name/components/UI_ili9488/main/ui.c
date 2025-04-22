//uses code from io_button repo's test code for GPIO buttons

#include "lvgl__lvgl/lvgl.h"
#include "iot_button.h"
#include "button_gpio.h"

//assorted graphic items used across functions
static lv_obj_t * tile1;
static lv_obj_t * tile2;
static lv_obj_t * tile3;
static button_handle_t down_btn;
static button_handle_t up_btn;

#include "helper_menus.c"

static const char *BUTTON = "button";

//callback function for the down button
static void down_btn_cb(void *arg,void *usr_data){
    lv_obj_t * currentTile = lv_scr_act();
    if(currentTile == tile1){
        loadTile2();
        lv_obj_del(tile1);
    }else if(currentTile == tile2){
        loadTile3();
        lv_obj_del(tile2);
    }
}

//callback function for the up button
static void up_btn_cb(void *arg,void *usr_data){
    lv_obj_t * currentTile = lv_scr_act();
    if(currentTile == tile2){
        loadTile1();
        lv_obj_del(tile2);
    }else if(currentTile == tile3){
        loadTile2();
        lv_obj_del(tile3);
    }
}

void initButtons(int pin, button_handle_t * name){
    // create gpio button
    const button_config_t btn_cfg = {0};
    const button_gpio_config_t gpio_btn_cfg = {
        .gpio_num = pin,
        .active_level = 0,
    };

    esp_err_t ret = iot_button_new_gpio_device(&btn_cfg, &gpio_btn_cfg, name);
    if(ret != ESP_OK) {
        ESP_LOGE(BUTTON, "Button create failed");
    }
}

void makeButtons(){
    initButtons(GPIO_DOWN_BUTTON, &down_btn);    
    initButtons(GPIO_UP_BUTTON, &up_btn);    
    //initButtons(GPIO_LEFT_BUTTON);    
    //initButtons(GPIO_RIGHT_BUTTON);    
    //initButtons(GPIO_SELECT_BUTTON);    
    iot_button_register_cb(down_btn, BUTTON_SINGLE_CLICK, NULL, down_btn_cb, NULL);
    iot_button_register_cb(down_btn, BUTTON_PRESS_REPEAT, NULL, down_btn_cb, NULL);
    iot_button_register_cb(up_btn, BUTTON_SINGLE_CLICK, NULL, up_btn_cb, NULL);
    iot_button_register_cb(up_btn, BUTTON_PRESS_REPEAT, NULL, up_btn_cb, NULL);
}

void create_ui(){
    initFonts();
    loadTile1();
    makeButtons();
}

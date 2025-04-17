#include "lvgl__lvgl/lvgl.h"

/* Text settings */
static lv_style_t style_text_muted;
static lv_style_t style_title;
static lv_style_t style_timer;
static const lv_font_t * font_large;
static const lv_font_t * font_giant;

static lv_obj_t * tileview;
static lv_obj_t * tile1;
static lv_obj_t * tile2;
static lv_obj_t * tile3;
static lv_obj_t * btn1;
static lv_obj_t * btn2;

static lv_obj_t * create_task(lv_obj_t * parent, const char * name, const char * dueDate)
{
    lv_obj_t * cont = lv_obj_class_create_obj(&lv_list_button_class, parent);
    lv_obj_class_init_obj(cont);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    
    static int32_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static int32_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

    lv_obj_set_grid_dsc_array(cont, grid_col_dsc, grid_row_dsc);
    lv_obj_set_style_pad_left(cont, 0, LV_PART_MAIN);

    lv_obj_t * label;
    label = lv_label_create(cont);
    lv_label_set_text_static(label, name);
    lv_obj_set_width(label, LCD_H_RES*0.4);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP); 
    lv_obj_set_grid_cell(label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_END, 0, 1);

    label = lv_label_create(cont);
    lv_label_set_text_static(label, dueDate);
    lv_obj_add_style(label, &style_text_muted, 0);
    lv_obj_set_grid_cell(label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 1, 1);

    return cont;
}

static void event_cb(lv_event_t * e){
    //scroll down to task menu
    lv_obj_t * target = lv_event_get_target(e);
    lv_obj_t * currentTile = lv_tileview_get_tile_act(tileview);
    lv_obj_t * label = lv_obj_get_child(target, 0); 
    if(target == btn1){
        if(currentTile == tile1){
            lv_label_set_text(label, "^");
            lv_tileview_set_tile(tileview, tile2, LV_ANIM_ON);
        }else if(currentTile == tile2){
            lv_label_set_text(label, "v");
            lv_tileview_set_tile(tileview, tile1, LV_ANIM_ON);
        }
    }else if(target == btn2){
        if(currentTile == tile2){
            lv_label_set_text(label, "^");
            lv_tileview_set_tile(tileview, tile3, LV_ANIM_ON);
        }else if(currentTile == tile3){
            lv_label_set_text(label, "v");
            lv_tileview_set_tile(tileview, tile2, LV_ANIM_ON);
        }
    }
}

/*
static void scroll_event_cb(lv_event_t * e){
    lv_obj_t * target = lv_event_get_target(e);
    lv_obj_t * currentTile = lv_tileview_get_tile_active(tileview);
    lv_obj_t * label1 = lv_obj_get_child(btn1, 0); 
    lv_obj_t * label2 = lv_obj_get_child(btn2, 0); 
    if(currentTile == tile1){
        lv_label_set_text(label1, "v");
    }else if(currentTile == tile2){
        lv_label_set_text(label1, "^");
    }

    if(currentTile == tile2){
        lv_label_set_text(label2, "v");
    }else if(currentTile == tile3){
        lv_label_set_text(label2, "^");
    }
}
*/

void taskEvent_create(lv_obj_t * parent){
    //TODO: needs time from RTC
    lv_obj_t * dateTime = lv_label_create(parent);
    lv_label_set_text(dateTime, "4/11/2025 7:35 PM");
    lv_obj_set_style_text_font(dateTime, &lv_font_montserrat_18, 0);
    lv_obj_align(dateTime, LV_ALIGN_TOP_LEFT, 5, 5);

    //make button for scroll down
    btn1 = lv_button_create(parent);
    lv_obj_set_size(btn1, 25, 25);
    lv_obj_align(btn1, LV_ALIGN_TOP_RIGHT, -5, 5);
    lv_obj_add_event_cb(btn1, event_cb, LV_EVENT_CLICKED, NULL);
    //lv_obj_set_style_bg_color(btn, lv_obj_get_style_bg_color(parent, LV_PART_MAIN), 0);

    lv_obj_t * label = lv_label_create(btn1);
    lv_label_set_text(label, "v");
    lv_obj_center(label);
    
    //create a title
    lv_obj_t * taskTitle = lv_label_create(parent);
    lv_label_set_text(taskTitle, "Tasks");
    lv_obj_align(taskTitle, LV_ALIGN_TOP_LEFT, 3, 35);

    //make tasklist 
    lv_obj_t * tasklist = lv_list_create(parent);
    lv_obj_set_size(tasklist, lv_pct(49), lv_pct(73));
    lv_obj_align(tasklist, LV_ALIGN_TOP_LEFT, 3, 50);
    lv_obj_set_style_pad_all(tasklist, 0, LV_PART_MAIN);
    
    create_task(tasklist, "Capstone Project", "3/25/2025");
    create_task(tasklist, "Figure out Prototype", "3/29/2025");
    create_task(tasklist, "Learn PCB Design", "3/30/2025");
    create_task(tasklist, "Learn Computer Aided Design", "4/1/2025");
    create_task(tasklist, "Do Mechatronics Lab", "4/1/2025");
    create_task(tasklist, "Meet with Professor John", "4/2/2025");
    create_task(tasklist, "Movie Night", "4/7/2025");
    create_task(tasklist, "Math homework", "4/8/2025");
    
    //create event list
    //create a title
    lv_obj_t * eventTitle = lv_label_create(parent);
    lv_label_set_text(eventTitle, "Events");
    lv_obj_align(eventTitle, LV_ALIGN_TOP_LEFT, (LCD_H_RES/2), 35);

    lv_obj_t * eventlist = lv_list_create(parent);
    lv_obj_set_size(eventlist, lv_pct(49), lv_pct(73));
    lv_obj_align(eventlist, LV_ALIGN_TOP_LEFT, (LCD_H_RES/2), 50);
    lv_obj_set_style_pad_all(eventlist, 0, LV_PART_MAIN);
    
    create_task(eventlist, "Capstone Meeting", "3/21/2025 3:00PM");
    create_task(eventlist, "ECE171 Class", "3/29/2025 2:00PM");
    create_task(eventlist, "CSE121 Class", "3/30/2025 5:00PM");
}


void focusMenu_create(lv_obj_t * parent){
    lv_obj_t * timer = lv_label_create(parent);
    lv_label_set_text(timer, "23:24");
    lv_obj_center(timer);
    lv_obj_add_style(timer, &style_timer, 0);

    lv_obj_t * task = lv_label_create(parent);
    lv_label_set_text(task, "current task");
    lv_obj_center(task);
    lv_obj_add_style(task, &style_title, 0);
    lv_obj_align(task, LV_ALIGN_CENTER, 0 , -75);

}

void habitMenu_create(lv_obj_t * parent){
    btn2 = lv_button_create(parent);
    lv_obj_set_size(btn2, 25, 25);
    lv_obj_align(btn2, LV_ALIGN_TOP_RIGHT, -5, 5);
    lv_obj_add_event_cb(btn2, event_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t * label = lv_label_create(btn2);
    lv_label_set_text(label, "v");
    lv_obj_center(label);

    /*
    lv_obj_t * habits = lv_label_create(parent);
    lv_label_set_text(habits, "Habit Progress");
    lv_obj_set_style_text_font(habits, &lv_font_montserrat_18, 0);
    lv_obj_align(habits, LV_ALIGN_TOP_LEFT, 5, 5);
    */
}
#include "lvgl.h"

void create_ui(){
    /*initialize fonts*/ 
    font_large     = &lv_font_montserrat_24;
    lv_style_set_text_font(&style_title, font_large);
    
    lv_style_init(&style_timer);
    font_giant     = &lv_font_montserrat_48;
    lv_style_set_text_font(&style_timer, font_giant);

    lv_style_init(&style_text_muted);
    lv_style_set_text_opa(&style_text_muted, LV_OPA_50);

    tileview = lv_tileview_create(lv_screen_active());
    //lv_screen_activelv_obj_add_event_cb(tileview, scroll_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF);
    //lv_gridnav_add(tileview, LV_GRIDNAV_CTRL_NONE);

    //create first tile
    tile1 = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_BOTTOM);
    
    //create blue color
    lv_color_t pastelBlue = lv_color_hex(0xcaf9ff); 
    lv_obj_set_style_bg_color(tile1, pastelBlue, 0);
    lv_obj_set_style_bg_opa(tile1, LV_OPA_100, 0);
    lv_obj_set_style_pad_all(tile1, 0, LV_PART_MAIN);
    focusMenu_create(tile1);
   
    //create second tile
    tile2 = lv_tileview_add_tile(tileview, 0, 1,(lv_dir_t)(LV_DIR_TOP | LV_DIR_BOTTOM));
    lv_obj_set_pos(tile2, lv_pct(0 * 90), lv_pct(1 * 90));

    //create green color
    lv_color_t pastelGreen = lv_color_hex(0xcaffe8); 
    lv_obj_set_style_bg_color(tile2, pastelGreen, 0);
    lv_obj_set_style_bg_opa(tile2, LV_OPA_100, 0);
    lv_obj_set_style_pad_all(tile2, 0, LV_PART_MAIN);
    taskEvent_create(tile2);

    //third tile
    tile3 = lv_tileview_add_tile(tileview, 0, 2, LV_DIR_TOP);
    lv_obj_set_pos(tile3, lv_pct(0 * 90), lv_pct(2 * 90));
    
    //create yellow color
    lv_color_t pastelYellow = lv_color_hex(0xdbffca); 
    lv_obj_set_style_bg_color(tile3, pastelYellow, 0);
    lv_obj_set_style_bg_opa(tile3, LV_OPA_100, 0);
    lv_obj_set_style_pad_all(tile3, 0, LV_PART_MAIN);
    habitMenu_create(tile3);
}

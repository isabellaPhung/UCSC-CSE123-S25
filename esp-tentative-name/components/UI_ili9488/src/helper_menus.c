#include "helper_menus.h"

sqlite3 *database;
static uint32_t taskCursor = 0;
static uint32_t eventCursor = 0;
static uint32_t habitCursor = 0;

/* Text settings */
static lv_style_t style_text_muted;
static lv_style_t style_title;
static lv_style_t style_timer;
static const lv_font_t * font_large;
static const lv_font_t * font_giant;

static lv_obj_t * tile1;
static lv_obj_t * tile2;
static lv_obj_t * tile3;
static lv_obj_t * taskTile;
static lv_obj_t * eventTile;
static lv_obj_t * tasklist;
static lv_obj_t * eventlist;
static lv_group_t * g1;

static lv_obj_t * arrowUp;
static lv_obj_t * arrowDown;
static lv_obj_t * button;
static lv_obj_t * title;
static lv_obj_t * label;
static lv_obj_t * dateTime;
static lv_obj_t * cont;
static lv_obj_t * child;
static lv_obj_t * obj;
static uint32_t k; //LVGL keyboard key

//static const char *TAG = "UI"; //for esp_log

void initDatabase(sqlite3 * db){
    database = db;
}

/*
 * callback function for focus menu for menu navigation
 * takes in keypress event 
 */
static void focus_cb(lv_event_t * e){
    k = lv_event_get_key(e);
    if(k == LV_KEY_DOWN) {
        loadTile2();
        lv_obj_del(tile1);
    }
}

/*
 * callback function for taskevent menu for menu navigation
 * takes in keypress event 
 */
static void taskevent_cb(lv_event_t * e){
    k = lv_event_get_key(e);
    if(k == LV_KEY_DOWN) {
        loadTile3();
        lv_obj_del(tile2);
    }else if(k == LV_KEY_UP) {
        loadTile1();
        lv_obj_del(tile2);
    }
}
/*
 * callback function for habit menu for menu navigation
 * takes in keypress event 
 */
static void habit_cb(lv_event_t * e){
    k = lv_event_get_key(e);
    if(k == LV_KEY_UP) {
        loadTile2();
        lv_obj_del(tile3);
    }else if(k == LV_KEY_LEFT){
        child = lv_obj_get_child(lv_event_get_target(e), 0);
        lv_group_focus_obj(child);
    }else if(k == LV_KEY_RIGHT){
        child = lv_obj_get_child(lv_event_get_target(e), 0);
        lv_group_focus_obj(child);
    }
}

/*
 * initializes necessary fonts
 * I would like this to be configurable
 * but maybe later
 */
void initFonts(){
     /*initialize fonts*/ 
    lv_style_init(&style_title);
    font_large     = &lv_font_montserrat_24;
    lv_style_set_text_font(&style_title, font_large);
    
    lv_style_init(&style_timer);
    font_giant     = &lv_font_montserrat_48;
    lv_style_set_text_font(&style_timer, font_giant);

    lv_style_init(&style_text_muted);
    lv_style_set_text_opa(&style_text_muted, LV_OPA_50);
}

/*
 * initializes necessary fonts
 */
void initGroup(){
    g1 = lv_group_create();
    lv_group_set_default(g1);
}

/*
 * updates clock display
 */
void timeDisplay(char * entry){
	if(lv_obj_is_valid(dateTime)){
    	lv_label_set_text(dateTime, entry);
	}
}

/*
 * creates focus menu, takes in parent lv_obj
 */
static void focusMenu_create(lv_obj_t * parent){
    //adding gridnav and focus colors to the taskevent menu
    lv_obj_set_style_bg_color(parent, lv_palette_lighten(LV_PALETTE_BLUE, 4), LV_STATE_FOCUSED);
    lv_gridnav_add(parent, LV_GRIDNAV_CTRL_VERTICAL_MOVE_ONLY);
    lv_group_add_obj(lv_group_get_default(), parent);
    lv_group_focus_obj(parent);
    lv_obj_add_event_cb(parent, focus_cb, LV_EVENT_KEY, NULL);

    //displays current task timer
    //TODO: what does it display if theres no current task? does it default to task tile?
    lv_obj_t * timer = lv_label_create(parent);
    lv_label_set_text(timer, "23:24");
    lv_obj_center(timer);
    lv_obj_add_style(timer, &style_timer, 0);

    //displays current task
    //TODO: how does the logic for displaying current task work?? need to ask Mason
    lv_obj_t * task = lv_label_create(parent);
    lv_label_set_text(task, "current task");
    lv_obj_center(task);
    lv_obj_add_style(task, &style_title, 0);
    lv_obj_align(task, LV_ALIGN_CENTER, 0 , -75);
    
    //adds time to focus tile
    dateTime = lv_label_create(parent);
    lv_obj_set_style_text_font(dateTime, &lv_font_montserrat_18, 0);
    lv_obj_align(dateTime, LV_ALIGN_BOTTOM_LEFT, 5, -5);

    //arrow to indicate scroll down
    arrowDown = lv_label_create(parent);
    lv_label_set_text(arrowDown, LV_SYMBOL_DOWN);
    lv_obj_align(arrowDown, LV_ALIGN_BOTTOM_RIGHT, -5, -5);
}

static void exit_event_cb(lv_event_t * e){
    loadTile2();
    lv_obj_del(eventTile);
}

static void delete_event_cb(lv_event_t * e){
    //TODO: delete event implementation
}

/* creates a new screen when event is selected and displays info */
static void eventTile_create(lv_obj_t * parent){
    //adding gridnav and focus colors to the taskevent menu
    cont = lv_obj_create(parent);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(cont, lv_pct(100), lv_pct(80));

    title = lv_label_create(cont);
    lv_label_set_text(title, "Event Title");
    lv_obj_add_style(title, &style_title, 0);
    
    label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_label_set_text(label, "Time and Date");

    label = lv_label_create(cont);
    lv_label_set_text(label, "Event Duration");

    label = lv_label_create(cont);
    lv_label_set_text(label, "Event Description");
    
    lv_obj_t * cont1 = lv_obj_create(parent);
    lv_obj_set_size(cont1, lv_pct(100), lv_pct(20));
    lv_obj_set_style_pad_all(cont1, 10, LV_PART_MAIN);
    lv_obj_align_to(cont1, cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_gridnav_add(cont1, LV_GRIDNAV_CTRL_HORIZONTAL_MOVE_ONLY);
    lv_group_add_obj(lv_group_get_default(), cont1);
    lv_obj_set_flex_flow(cont1, LV_FLEX_FLOW_ROW);

    button = lv_btn_create(cont1);
    lv_group_remove_obj(button);
    lv_obj_add_event_cb(button, exit_event_cb, LV_EVENT_CLICKED, NULL);
    label = lv_label_create(button);
    lv_label_set_text(label, "Exit");
    lv_obj_center(label);
    lv_group_focus_obj(button);

    button = lv_btn_create(cont1);
    lv_group_remove_obj(button);
    lv_obj_add_event_cb(button, delete_event_cb, LV_EVENT_CLICKED, NULL);
    label = lv_label_create(button);
    lv_label_set_text(label, "Delete");
    lv_obj_center(label);

}

/*
 * loads new screen for event
 */
void loadEventTile(){
    eventTile = lv_obj_create(NULL);
    lv_scr_load(eventTile); 
    eventTile_create(eventTile);
}

static void event_desc_cb(lv_event_t * e){
    loadEventTile();
    lv_obj_del(tile2);
}


static void exit_task_cb(lv_event_t * e){
    loadTile2();
    lv_obj_del(taskTile);
}

static void complete_task_cb(lv_event_t * e){
    //TODO: task completion implementation
}

static void focus_task_cb(lv_event_t * e){
    //TODO: focus task implementation
}

static void delete_task_cb(lv_event_t * e){
    //TODO: delte task implementation
}

/* creates a new screen when Tile is selected and displays info */
static void taskTile_create(lv_obj_t * parent){
    //adding gridnav and focus colors to the taskevent menu
    cont = lv_obj_create(parent);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(cont, lv_pct(100), lv_pct(80));

    title = lv_label_create(cont);
    lv_label_set_text(title, "Task Title");
    lv_obj_add_style(title, &style_title, 0);
    
    label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_label_set_text(label, "Due date");

    label = lv_label_create(cont);
    lv_label_set_text(label, "Task Priority");

    label = lv_label_create(cont);
    lv_label_set_text(label, "Task Description");
    
    lv_obj_t * cont1 = lv_obj_create(parent);
    lv_obj_set_size(cont1, lv_pct(100), lv_pct(20));
    lv_obj_set_style_pad_all(cont1, 10, LV_PART_MAIN);
    lv_obj_align_to(cont1, cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_gridnav_add(cont1, LV_GRIDNAV_CTRL_HORIZONTAL_MOVE_ONLY);
    lv_group_add_obj(lv_group_get_default(), cont1);
    lv_obj_set_flex_flow(cont1, LV_FLEX_FLOW_ROW);

    button = lv_btn_create(cont1);
    lv_group_remove_obj(button);
    lv_obj_add_event_cb(button, exit_task_cb, LV_EVENT_CLICKED, NULL);
    label = lv_label_create(button);
    lv_label_set_text(label, "Exit");
    lv_obj_center(label);
    lv_group_focus_obj(button);
    
    button = lv_btn_create(cont1);
    lv_group_remove_obj(button);
    lv_obj_add_event_cb(button, focus_task_cb, LV_EVENT_CLICKED, NULL);
    label = lv_label_create(button);
    lv_label_set_text(label, "Focus Task");
    lv_obj_center(label);
    
    button = lv_btn_create(cont1);
    lv_group_remove_obj(button);
    lv_obj_add_event_cb(button, complete_task_cb, LV_EVENT_CLICKED, NULL);
    label = lv_label_create(button);
    lv_label_set_text(label, "Task Completed");
    lv_obj_center(label);

    button = lv_btn_create(cont1);
    lv_group_remove_obj(button);
    lv_obj_add_event_cb(button, delete_task_cb, LV_EVENT_CLICKED, NULL);
    label = lv_label_create(button);
    lv_label_set_text(label, "Delete");
    lv_obj_center(label);

}

/*
 * loads new screen for task
 */
void loadTaskTile(){
    taskTile = lv_obj_create(NULL);
    lv_scr_load(taskTile); 
    taskTile_create(taskTile);
}

static void task_desc_cb(lv_event_t * e){
    loadTaskTile();
    lv_obj_del(tile2);
}

/*
 * takes parent obj(usually an lv list), name of task, and due date of task
 * returns an lv_obj pointer to task list entry
 * probably needs to be fixed to be generic for tasks
 */
void create_task(task_t * task){
    //creates button for task using existing list button style
    cont = lv_obj_class_create_obj(&lv_list_button_class, tasklist);
    lv_obj_class_init_obj(cont);
    //lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN); //sets button to flex flow column form so it'll expand as needed
    lv_group_remove_obj(cont);   //Not needed, we use the gridnav instead
    lv_obj_add_event_cb(cont, task_desc_cb, LV_EVENT_CLICKED, &task);
   
    //initalizes columns and rows of grid for the button so things are nicely aligned
    static int32_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static int32_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    
    //initializes grid
    lv_obj_set_grid_dsc_array(cont, grid_col_dsc, grid_row_dsc);
    lv_obj_set_style_pad_left(cont, 0, LV_PART_MAIN); //removes unnecessary left margin

    //label for task name
    lv_obj_t * label;
    label = lv_label_create(cont);
    lv_label_set_text(label, task->name);
    lv_obj_set_width(label, LCD_H_RES*0.4);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP); 
    lv_obj_set_grid_cell(label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_END, 0, 1);

    //label for due date
    label = lv_label_create(cont);
    struct tm * timeinfo = gmtime(&(task->time));
    char timestr[40];
    strftime(timestr, sizeof(timestr), "%D %r", timeinfo);
    lv_label_set_text(label, timestr);
    lv_obj_add_style(label, &style_text_muted, 0);
    lv_obj_set_grid_cell(label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 1, 1);
}

/*
 * takes parent obj(usually an lv list), name of task, and due date of task
 * returns an lv_obj pointer to task list entry
 * probably needs to be fixed to be generic for tasks
 */
void create_event(event_t * event){
    //creates button for task using existing list button style
    cont = lv_obj_class_create_obj(&lv_list_button_class, eventlist);
    lv_obj_class_init_obj(cont);
    lv_group_remove_obj(cont);   //Not needed, we use the gridnav instead
    lv_obj_add_event_cb(cont, event_desc_cb, LV_EVENT_CLICKED, &event);
   
    //initalizes columns and rows of grid for the button so things are nicely aligned
    static int32_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static int32_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    
    //initializes grid
    lv_obj_set_grid_dsc_array(cont, grid_col_dsc, grid_row_dsc);
    lv_obj_set_style_pad_left(cont, 0, LV_PART_MAIN); //removes unnecessary left margin

    //label for task name
    lv_obj_t * label;
    label = lv_label_create(cont);
    lv_label_set_text(label, event->name);
    lv_obj_set_width(label, LCD_H_RES*0.4);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP); 
    lv_obj_set_grid_cell(label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_END, 0, 1);

    //label for due date
    label = lv_label_create(cont);
    struct tm * timeinfo = gmtime(&(event->start_time));
    char timestr[40];
    strftime(timestr, sizeof(timestr), "%D %r", timeinfo);
    lv_label_set_text(label, timestr);
    lv_obj_add_style(label, &style_text_muted, 0);
    lv_obj_set_grid_cell(label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 1, 1);
}

task_t * taskBuffer;
event_t * eventBuffer;
habit_t * habitBuffer;
void initBuffers(){
    taskBuffer = (task_t *)malloc(sizeof(task_t)*4);
    eventBuffer = (event_t *)malloc(sizeof(event_t)*4);
    habitBuffer = (habit_t *)malloc(sizeof(habit_t)*3);
}

static void tasks_left_cb(){
    if(eventCursor >= 4){
        taskCursor -= 4;
        int taskNum = RetrieveTasksSortedDB(database, taskBuffer, 4, taskCursor);
        for(int i = 0; i < taskNum; i++){
            create_task(&taskBuffer[i]);
        }
    }
}

static void tasks_right_cb(){
    int taskNum = RetrieveTasksSortedDB(database, taskBuffer, 4, taskCursor);
    if(taskNum == 4){
        taskCursor += 4;
    }
    for(int i = 0; i < taskNum; i++){
        create_task(&taskBuffer[i]);
    }
}

static void events_left_cb(){
    if(eventCursor >= 4){
        eventCursor -= 4;
        int eventNum = RetrieveEventsSortedDB(database, eventBuffer, 4, eventCursor);
        for(int i = 0; i < eventNum; i++){
            create_event(&eventBuffer[i]);
        }
    }
}

static void events_right_cb(){
    int eventNum = RetrieveEventsSortedDB(database, eventBuffer, 4, eventCursor);
    if(eventNum == 4){
        eventCursor += 4;
    }
    for(int i = 0; i < eventNum; i++){
        create_event(&eventBuffer[i]);
    }
}

static void button_nav_cb(lv_event_t * e){
    k = lv_event_get_key(e);
    obj = lv_event_get_target(e);
    if(k == LV_KEY_RIGHT) {
        lv_group_focus_next(lv_obj_get_group(obj));
    }else if(k == LV_KEY_LEFT) {
        lv_group_focus_prev(lv_obj_get_group(obj));
    }
}

/*
 * creates menu and lists for tasklist and event list
 * takes in parent lv_obj
 */
static void taskEvent_create(lv_obj_t * parent){
    //adding gridnav and focus colors to the taskevent menu
    lv_obj_set_style_bg_color(parent, lv_palette_lighten(LV_PALETTE_BLUE, 4), LV_STATE_FOCUSED);
    lv_gridnav_add(parent, LV_GRIDNAV_CTRL_NONE);
    lv_group_add_obj(lv_group_get_default(), parent);
    lv_group_focus_obj(parent);
    lv_obj_add_event_cb(parent, taskevent_cb, LV_EVENT_KEY, NULL);
    
    //TODO: needs time from RTC
    //adds time to taskevent tile
    dateTime = lv_label_create(parent);
    lv_obj_set_style_text_font(dateTime, &lv_font_montserrat_18, 0);
    lv_obj_align(dateTime, LV_ALIGN_TOP_LEFT, 5, 5);

    //arrow to indicate scroll up
    arrowUp = lv_label_create(parent);
    lv_label_set_text(arrowUp, LV_SYMBOL_UP);
    lv_obj_align(arrowUp, LV_ALIGN_TOP_RIGHT, -5, 5);
    
    //left task list arrows
    button = lv_btn_create(parent);
    lv_group_remove_obj(button);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, (LCD_H_RES/4)-25, 35);
    lv_obj_set_size(button, 25, 25);
    lv_obj_add_event_cb(button, tasks_left_cb, LV_EVENT_CLICKED, NULL);
    label = lv_label_create(button);
    lv_label_set_text(label, LV_SYMBOL_LEFT);
    lv_obj_center(label);
    
    //right task list arrows
    button = lv_btn_create(parent);
    lv_group_remove_obj(button);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, (LCD_H_RES/4)+25, 35);
    lv_obj_set_size(button, 25, 25);
    lv_obj_add_event_cb(button, tasks_right_cb, LV_EVENT_CLICKED, NULL);
    label = lv_label_create(button);
    lv_label_set_text(label, LV_SYMBOL_RIGHT);
    lv_obj_center(label);

    //left event list arrows
    button = lv_btn_create(parent);
    lv_group_remove_obj(button);
    lv_obj_align(button, LV_ALIGN_TOP_RIGHT, -((LCD_H_RES/4)+25), 35);
    lv_obj_add_event_cb(button, events_left_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_size(button, 25, 25);
    label = lv_label_create(button);
    lv_label_set_text(label, LV_SYMBOL_LEFT);
    lv_obj_center(label);
    
    //right event list arrows
    button = lv_btn_create(parent);
    lv_group_remove_obj(button);
    lv_obj_align(button, LV_ALIGN_TOP_RIGHT, -((LCD_H_RES/4)-25), 35);
    lv_obj_add_event_cb(button, events_right_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(button, 25, 25);
    label = lv_label_create(button);
    lv_label_set_text(label, LV_SYMBOL_RIGHT);
    lv_obj_center(label);

    //make tasklist 
    //create lv list obj
    tasklist = lv_list_create(parent);
    lv_group_add_obj(lv_group_get_default(), tasklist);
    lv_gridnav_add(tasklist, LV_GRIDNAV_CTRL_NONE);
    lv_obj_set_size(tasklist, lv_pct(49), lv_pct(73));
    lv_obj_align(tasklist, LV_ALIGN_TOP_LEFT, 3, 60);
    lv_obj_set_style_pad_all(tasklist, 0, LV_PART_MAIN);

    //create a title for tasks
    title = lv_label_create(parent);
    lv_label_set_text(title, "Tasks");
    lv_obj_align(title, LV_ALIGN_BOTTOM_LEFT, 5, -5);
    
    //create event list
    //create lv list obj
    eventlist = lv_list_create(parent);
    lv_group_add_obj(lv_group_get_default(), eventlist);
    lv_gridnav_add(eventlist, LV_GRIDNAV_CTRL_NONE);
    lv_obj_set_size(eventlist, lv_pct(49), lv_pct(73));
    lv_obj_align(eventlist, LV_ALIGN_TOP_LEFT, (LCD_H_RES/2), 60);
    lv_obj_set_style_pad_all(eventlist, 0, LV_PART_MAIN);
  
     //create a title for events
    title = lv_label_create(parent);
    lv_label_set_text(title, "Events");
    lv_obj_align(title, LV_ALIGN_BOTTOM_LEFT, (LCD_H_RES/2)+3, -5);

    //arrow to indicate scroll down
    arrowDown = lv_label_create(parent);
    lv_label_set_text(arrowDown, LV_SYMBOL_DOWN);
    lv_obj_align(arrowDown, LV_ALIGN_BOTTOM_RIGHT, -5, -5);
    
    tasks_right_cb();
    events_right_cb();
    /*
    //dummy tasks and events for testing
    create_task("Capstone Project", "3/25/2025");
    create_task("Figure out Prototype", "3/29/2025");
    create_task("Learn PCB Design", "3/30/2025");
    create_task("Learn Computer Aided Design", "4/1/2025");
    create_event("Capstone Meeting", "3/21/2025 3:00PM");
    create_event("ECE171 Class", "3/29/2025 2:00PM");
    create_event("CSE121 Class", "3/30/2025 5:00PM");
    */
}

/*
 * creates callback function for habit buttons
 * takes in event which is the keypress
 */
static void buttonmatrix_cb(lv_event_t * e){
    k = lv_event_get_key(e);
    obj = lv_event_get_target(e);
    if(k == LV_KEY_UP) {
        lv_group_focus_prev(lv_obj_get_group(obj));
    }else if(k == LV_KEY_DOWN) {
        lv_group_focus_next(lv_obj_get_group(obj));
    }else if(k == LV_KEY_ENTER) {
        //TODO: set habit day as updated
    }
}

static const char * btnm_map[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa", ""};
/*
 * creates habit entry for habit list
 * takes in parent list and name of habit and the row number.
 */
void createHabit(habit_t * habit, uint8_t row){
    //creates habit title name
    lv_obj_t * habits = lv_label_create(tile3);
    lv_label_set_text(habits, habit->name);
    lv_obj_set_style_text_font(habits, &lv_font_montserrat_18, 0);
    lv_obj_set_pos(habits, 15, 25+(row*100));
    
    //trying out button matrix
    lv_obj_t * buttons = lv_btnmatrix_create(tile3);
    lv_gridnav_add(buttons, LV_GRIDNAV_CTRL_NONE);
    lv_obj_add_event_cb(buttons, buttonmatrix_cb, LV_EVENT_KEY, NULL);
    lv_obj_set_style_bg_color(tile3, lv_palette_lighten(LV_PALETTE_BLUE, 4), LV_STATE_FOCUSED);
    lv_obj_set_style_pad_all(buttons, 5, LV_PART_MAIN);
    lv_group_add_obj(lv_group_get_default(), buttons);
    lv_btnmatrix_set_map(buttons, btnm_map);
    lv_obj_set_pos(buttons, 15, 50+(row*100));
    lv_obj_set_size(buttons, LCD_H_RES-50, LCD_V_RES/5.5);
    for(uint8_t i = 0; i < 7; i++){
        lv_btnmatrix_set_btn_ctrl(buttons, i, LV_BTNMATRIX_CTRL_CHECKABLE);
        //TODO: disable button if not necessary for that day or toggle based off of habit list
        //lv_obj_add_state(btn, LV_STATE_DISABLED);
    }
}

static void habits_left_cb(lv_event_t * e){
        //TODO: load prev 4 habits
}

static void habits_right_cb(lv_event_t * e){
        //TODO: load next 4 habits
}

/*
 * creates habit menu, takes in parent lv obj
 */
static void habitMenu_create(lv_obj_t * parent){
    //and color when focused and add gridnav 
    lv_obj_set_style_bg_color(parent, lv_palette_lighten(LV_PALETTE_BLUE, 4), LV_STATE_FOCUSED);
    lv_gridnav_add(parent, LV_GRIDNAV_CTRL_NONE);
    lv_group_add_obj(lv_group_get_default(), parent);
    lv_group_focus_obj(parent);
    lv_obj_add_event_cb(parent, habit_cb, LV_EVENT_KEY, NULL);

    //left habit list arrows
    button = lv_btn_create(parent);
    lv_group_remove_obj(button);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, +25, -35);
    lv_obj_add_event_cb(button, habits_left_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(button, 25, 25);
    label = lv_label_create(button);
    lv_label_set_text(label, LV_SYMBOL_LEFT);
    lv_obj_center(label);
    
    //right habit list arrows
    button = lv_btn_create(parent);
    lv_group_remove_obj(button);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, -25, -35);
    lv_obj_add_event_cb(button, habits_right_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(button, 25, 25);
    label = lv_label_create(button);
    lv_label_set_text(label, LV_SYMBOL_RIGHT);
    lv_obj_center(label);

    //arrow to indicate scroll up
    arrowUp = lv_label_create(parent);
    lv_label_set_text(arrowUp, LV_SYMBOL_UP);
    lv_obj_align(arrowUp, LV_ALIGN_TOP_RIGHT, -5, 5);

    //creates a title for the habits tile
    title = lv_label_create(parent);
    lv_label_set_text(title, "Habit Progress");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 5, 5);

    /*
    //dummy habits
    createHabit("Go to the Gym", 0);
    createHabit("Walk the dog", 1);
    createHabit("Journal", 2);
    */
}

/*
 * loads 1st tile, Focus
 */
void loadTile1(){
    tile1 = lv_obj_create(NULL);
    lv_scr_load(tile1); 
    focusMenu_create(tile1);
}

/*
 * loads 2nd tile, Task Tile
 */
void loadTile2(){
    tile2 = lv_obj_create(NULL);
    lv_scr_load(tile2); 
    taskEvent_create(tile2);
}

/*
 * loads 3rd tile, Habits Tile
 */
void loadTile3(){
    tile3 = lv_obj_create(NULL);
    lv_scr_load(tile3); 
    habitMenu_create(tile3);
}

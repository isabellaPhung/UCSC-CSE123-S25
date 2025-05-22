#include "helper_menus.h"
//#include "esp_log.h"
//#include "esp_system.h"

//sqlite3 *database;
static uint32_t taskCursor = 0;
static uint32_t eventCursor = 0;
static uint32_t habitCursor = 0;

/* Text settings */
static lv_style_t style_text_muted;
static lv_style_t style_title;
static lv_style_t style_timer;
static const lv_font_t * font_large;
static const lv_font_t * font_giant;

static lv_obj_t * tile2;
static lv_obj_t * tile3;
static lv_obj_t * taskTile;
static lv_obj_t * focusTile;
static lv_obj_t * wifiTile;
static lv_obj_t * loadingMsg;
static lv_obj_t * eventTile;
static lv_obj_t * tasklist;
static lv_obj_t * eventlist;
static lv_obj_t * habitlist;
static lv_obj_t * timer;
static lv_group_t * g1;

static lv_obj_t * arrowUp;
static lv_obj_t * arrowDown;
static lv_obj_t * button;
static lv_obj_t * buttonCont;
static lv_obj_t * title;
static lv_obj_t * label;
static lv_obj_t * wifiSymbol;
static lv_obj_t * dateTime;
static lv_obj_t * cont;
static uint32_t k; //LVGL keyboard key

task_t taskBuffer[4];
uint8_t taskBuffSize;
event_t eventBuffer[4];
uint8_t eventBuffSize;
habit_t habitBuffer[3];
uint8_t habitBuffSize;

time_t startTime;

//static const char *TAG = "UI"; //for esp_log

/*
char * convertTime(time_t * value){
    struct tm * timeinfo = localtime(value);
    char timestr[40];
    strftime(timestr, sizeof(timestr), "%D %r", timeinfo);
    return timestr;
}

void initDatabase(sqlite3 * db){
    database = db;
}
*/


/*
 * callback function for focus menu for menu navigation
 * takes in keypress event 
 */
static void focus_cb(lv_event_t * e){
    k = lv_event_get_key(e);
    //TODO
}

/*
 * callback function for taskevent menu for menu navigation
 * takes in keypress event 
 */
static void taskevent_cb(lv_event_t * e){
    k = lv_event_get_key(e);
    if(k == LV_KEY_DOWN) {
        lv_obj_clean(tile2);
        loadTile3();
    }
}

/*
 * callback function for habit menu for menu navigation
 * takes in keypress event 
 */
static void habit_cb(lv_event_t * e){
    k = lv_event_get_key(e);
    if(k == LV_KEY_UP) {
        lv_obj_clean(tile3);
        loadTile2();
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
	if((lv_obj_is_valid(dateTime) && lv_scr_act() == tile2) || (lv_obj_is_valid(dateTime) && lv_scr_act() == focusTile)){
    	lv_label_set_text(dateTime, entry);
	}
}

/*
 * updates clock display
 */
void wifiDisplay(bool hasWifi){
    if(lv_obj_is_valid(wifiSymbol)){
        if(hasWifi){
            lv_obj_clear_flag(wifiSymbol, LV_OBJ_FLAG_HIDDEN);
        }else{
            lv_obj_add_flag(wifiSymbol, LV_OBJ_FLAG_HIDDEN);
        }
    }
    
}

static void exit_task_cb(){
    loadTile2();
}

static void complete_task_cb(lv_event_t * e){
    task_t * task = lv_event_get_user_data(e);
    UpdateTaskStatus(task->uuid, COMPLETE);
    exit_task_cb();
}

bool isFocusMode(){
    return lv_scr_act() == focusTile;
}

void resetTimer(){
    startTime = time(NULL);
}

void updateFocusTimer(){
    if(lv_scr_act() == focusTile && lv_obj_is_valid(timer)){
        time_t currTime = time(NULL);
        time_t difference = (time_t)1800 - difftime(currTime, startTime);
        int mins = 0;
        int secs = 0;
        if(difference > 0){ //while 30 mins haven't passed
            mins = difference/60;
            secs = difference%60;
        }
        //displays current task timer
        lv_label_set_text_fmt(timer, "%02d:%02d", mins, secs);
    }
}
/*
 * creates focus menu, takes in parent lv_obj
 */
static void focusTile_create(lv_obj_t * parent, task_t * task){
    //adding gridnav and focus colors to the taskevent menu
    lv_obj_add_event_cb(parent, focus_cb, LV_EVENT_KEY, NULL);

    updateFocusTimer();    
    
    //displays current task timer
    timer = lv_label_create(parent);
    lv_label_set_text(timer, "00:00");
    lv_obj_center(timer);
    lv_obj_add_style(timer, &style_timer, 0);

    //displays current task
    lv_obj_t * taskName = lv_label_create(parent);
    lv_label_set_text_static(taskName, task->name);
    lv_obj_center(taskName);
    lv_obj_add_style(taskName, &style_title, 0);
    lv_obj_align(taskName, LV_ALIGN_CENTER, 0 , -75);
    
    //adds time to focus tile
    dateTime = lv_label_create(parent);
    lv_obj_set_style_text_font(dateTime, &lv_font_montserrat_18, 0);
    lv_label_set_text(dateTime, "loading...");
    lv_obj_align(dateTime, LV_ALIGN_TOP_LEFT, 5, 5);
    
    lv_obj_t * cont1 = lv_obj_create(parent);
    lv_obj_set_size(cont1, lv_pct(100), lv_pct(20));
    lv_obj_set_style_pad_all(cont1, 10, LV_PART_MAIN);
    lv_obj_align_to(cont1, cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_gridnav_add(cont1, LV_GRIDNAV_CTRL_HORIZONTAL_MOVE_ONLY);
    lv_group_add_obj(lv_group_get_default(), cont1);
    lv_obj_set_flex_flow(cont1, LV_FLEX_FLOW_ROW);
    lv_group_focus_obj(cont1);
    
    button = lv_btn_create(cont1);
    lv_group_remove_obj(button);
    lv_obj_add_event_cb(button, exit_task_cb, LV_EVENT_CLICKED, NULL);
    label = lv_label_create(button);
    lv_label_set_text(label, "Exit");
    lv_obj_center(label);

    button = lv_btn_create(cont1);
    lv_group_remove_obj(button);
    lv_obj_add_event_cb(button, resetTimer, LV_EVENT_CLICKED, task);
    label = lv_label_create(button);
    lv_label_set_text(label, "Reset Timer");
    lv_obj_center(label);

    button = lv_btn_create(cont1);
    lv_group_remove_obj(button);
    lv_obj_add_event_cb(button, complete_task_cb, LV_EVENT_CLICKED, task);
    label = lv_label_create(button);
    lv_label_set_text(label, "Task Complete");
    lv_obj_center(label);
}

/*
 * loads 1st tile, Focus
 */
void loadFocusTile(task_t * task){
    focusTile = lv_obj_create(NULL);
    lv_scr_load(focusTile); 
    focusTile_create(focusTile, task);
}

static void focus_task_cb(lv_event_t * e){
    resetTimer();
    loadFocusTile(lv_event_get_user_data(e));
}

static void exit_event_cb(){
    loadTile2();
}

static void delete_event_cb(lv_event_t * e){
    event_t * event = lv_event_get_user_data(e);
    RemoveEventDB(event->uuid);
    exit_event_cb();
}

/* creates a new screen when event is selected and displays info */
static void eventTile_create(lv_obj_t * parent, event_t * event){
    //adding gridnav and focus colors to the taskevent menu
    cont = lv_obj_create(parent);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(cont, lv_pct(100), lv_pct(80));

    title = lv_label_create(cont);
    lv_label_set_text_static(title, event->name);
    lv_obj_add_style(title, &style_title, 0);
    
    label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    struct tm * timeinfo = localtime(&(event->start_time));
    char timestr[40];
    strftime(timestr, sizeof(timestr), "%D %r", timeinfo);
    lv_label_set_text_fmt(label, "Date: %s", timestr);

    label = lv_label_create(cont);
    timeinfo = localtime(&(event->duration));
    strftime(timestr, sizeof(timestr), "%I:%M:%S", timeinfo);
    lv_label_set_text_fmt(label, "Duration: %s", timestr);

    label = lv_label_create(cont);
    lv_label_set_text_static(label, event->description);
    
    lv_obj_t * cont1 = lv_obj_create(parent);
    lv_obj_set_size(cont1, lv_pct(100), lv_pct(20));
    lv_obj_set_style_pad_all(cont1, 10, LV_PART_MAIN);
    lv_obj_align_to(cont1, cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_gridnav_add(cont1, LV_GRIDNAV_CTRL_HORIZONTAL_MOVE_ONLY);
    lv_group_focus_obj(cont1);
    lv_group_add_obj(lv_group_get_default(), cont1);
    lv_obj_set_flex_flow(cont1, LV_FLEX_FLOW_ROW);

    button = lv_btn_create(cont1);
    lv_group_remove_obj(button);
    lv_obj_add_event_cb(button, exit_event_cb, LV_EVENT_CLICKED, NULL);
    label = lv_label_create(button);
    lv_label_set_text(label, "Exit");
    lv_obj_center(label);

    button = lv_btn_create(cont1);
    lv_group_remove_obj(button);
    lv_obj_add_event_cb(button, delete_event_cb, LV_EVENT_CLICKED, event);
    label = lv_label_create(button);
    lv_label_set_text(label, "Delete");
    lv_obj_center(label);

}

void loadMsgCreate(void){
    loadingMsg = lv_msgbox_create(NULL);
    lv_msgbox_add_title(loadingMsg, "Loading");
    lv_msgbox_add_text(loadingMsg, "Loading, please wait.");

    lv_group_focus_freeze(lv_group_get_default(), true);

    lv_obj_align(loadingMsg, LV_ALIGN_CENTER, 0, 0);
}

void loadMsgRemove(void){
    if(lv_obj_is_valid(loadingMsg)){
        lv_msgbox_close(loadingMsg);
        lv_group_focus_freeze(lv_group_get_default(), false);
    }
}

void wifiTile_create(lv_obj_t * parent){
    label = lv_label_create(parent);
    lv_label_set_text_fmt(label, "Please connect to the device hotspot and set up the Wifi SSID and password!\nAccess Point Name: %s\nAccess Point Password: %s", CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
    //lv_label_set_text_fmt(label, "Please connect to the device hotspot and set up the Wifi SSID and password!");
    lv_obj_add_style(label, &style_title, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0 , -50);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_width(label, LCD_H_RES-100);

    wifiSymbol = lv_label_create(parent);
    lv_label_set_text(wifiSymbol, LV_SYMBOL_WIFI);
    lv_obj_add_style(wifiSymbol, &style_timer, 0);
    lv_obj_align_to(wifiSymbol, label, LV_ALIGN_OUT_BOTTOM_MID, 0 , 20);
}

/*
 * Displays wifi info if not connected
 */
void loadWifiTile(){
    wifiTile = lv_obj_create(NULL);
    lv_scr_load(wifiTile); 
    wifiTile_create(wifiTile);
}

/*
 * loads new screen for event
 */
void loadEventTile(event_t * event){
    eventTile = lv_obj_create(NULL);
    lv_scr_load(eventTile); 
    eventTile_create(eventTile, event);
}

static void event_desc_cb(lv_event_t * e){
    loadEventTile(lv_event_get_user_data(e));
}

static void incomplete_task_cb(lv_event_t * e){
    task_t * task = lv_event_get_user_data(e);
    UpdateTaskStatus(task->uuid, INCOMPLETE);
    exit_task_cb();
}

static void delete_task_cb(lv_event_t * e){
    task_t * task = lv_event_get_user_data(e);
    UpdateTaskStatus(task->uuid, MFD);
    exit_task_cb();
}

/* creates a new screen when Tile is selected and displays info */
static void taskTile_create(lv_obj_t * parent, task_t * task){
    //adding gridnav and focus colors to the taskevent menu
    cont = lv_obj_create(parent);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(cont, lv_pct(100), lv_pct(80));

    title = lv_label_create(cont);
    lv_label_set_text_static(title, task->name);
    lv_obj_add_style(title, &style_title, 0);
    
    label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    struct tm * timeinfo = localtime(&(task->time));
    char timestr[40];
    strftime(timestr, sizeof(timestr), "%D %r", timeinfo);
    lv_label_set_text_fmt(label, "Due date: %s", timestr);
   
    label = lv_label_create(cont);
    if(task->completion == 0){
        lv_label_set_text(label, "Status: Incomplete");
    }else if(task->completion == 1){
        lv_label_set_text(label, "Status: Complete");
    }

    label = lv_label_create(cont);
    lv_label_set_text_fmt(label, "Priority: %d", task->priority);

    label = lv_label_create(cont);
    lv_label_set_text_static(label, task->description);
    
    lv_obj_t * cont1 = lv_obj_create(parent);
    lv_obj_set_size(cont1, lv_pct(100), lv_pct(20));
    lv_obj_set_style_pad_all(cont1, 10, LV_PART_MAIN);
    lv_obj_align_to(cont1, cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_gridnav_add(cont1, LV_GRIDNAV_CTRL_HORIZONTAL_MOVE_ONLY);
    lv_group_add_obj(lv_group_get_default(), cont1);
    lv_obj_set_flex_flow(cont1, LV_FLEX_FLOW_ROW);
    lv_group_focus_obj(cont1);

    button = lv_btn_create(cont1);
    lv_group_remove_obj(button);
    lv_obj_add_event_cb(button, exit_task_cb, LV_EVENT_CLICKED, NULL);
    label = lv_label_create(button);
    lv_label_set_text(label, "Exit");
    lv_obj_center(label);
   
    button = lv_btn_create(cont1);
    lv_group_remove_obj(button);
    if(task->completion == INCOMPLETE){
        lv_obj_add_event_cb(button, focus_task_cb, LV_EVENT_CLICKED, task);
        label = lv_label_create(button);
        lv_label_set_text(label, "Focus Task");
        lv_obj_center(label);

        button = lv_btn_create(cont1);
        lv_group_remove_obj(button);
        lv_obj_add_event_cb(button, complete_task_cb, LV_EVENT_CLICKED, task);
        label = lv_label_create(button);
        lv_label_set_text(label, "Task Complete");
        lv_obj_center(label);
    }else{
        lv_obj_add_event_cb(button, incomplete_task_cb, LV_EVENT_CLICKED, task);
        label = lv_label_create(button);
        lv_label_set_text(label, "Task Incomplete");
        lv_obj_center(label);
    }
    
    button = lv_btn_create(cont1);
    lv_group_remove_obj(button);
    lv_obj_add_event_cb(button, delete_task_cb, LV_EVENT_CLICKED, task);
    label = lv_label_create(button);
    lv_label_set_text(label, "Delete");
    lv_obj_center(label);

}

/*
 * loads new screen for task
 */
void loadTaskTile(task_t * task){
    taskTile = lv_obj_create(NULL);
    lv_scr_load(taskTile); 
    taskTile_create(taskTile, task);
}

static void task_desc_cb(lv_event_t * e){
    loadTaskTile(lv_event_get_user_data(e));
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
    lv_obj_add_event_cb(cont, task_desc_cb, LV_EVENT_CLICKED, task);
    if(task->completion == COMPLETE){
        lv_obj_set_style_bg_color(cont, lv_palette_lighten(LV_PALETTE_GREEN, 5), LV_PART_MAIN);
    }
  
    //initalizes columns and rows of grid for the button so things are nicely aligned
    static int32_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static int32_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    
    //initializes grid
    lv_obj_set_grid_dsc_array(cont, grid_col_dsc, grid_row_dsc);
    lv_obj_set_style_pad_left(cont, 0, LV_PART_MAIN); //removes unnecessary left margin

    //label for task name
    lv_obj_t * label;
    label = lv_label_create(cont);
    lv_label_set_text_static(label, task->name);
    lv_obj_set_width(label, LCD_H_RES*0.4);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP); 
    lv_obj_set_grid_cell(label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_END, 0, 1);

    //label for due date
    label = lv_label_create(cont);
    struct tm * timeinfo = localtime(&(task->time));
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
    lv_obj_add_event_cb(cont, event_desc_cb, LV_EVENT_CLICKED, event);
   
    //initalizes columns and rows of grid for the button so things are nicely aligned
    static int32_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static int32_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    
    //initializes grid
    lv_obj_set_grid_dsc_array(cont, grid_col_dsc, grid_row_dsc);
    lv_obj_set_style_pad_left(cont, 0, LV_PART_MAIN); //removes unnecessary left margin

    //label for task name
    lv_obj_t * label;
    label = lv_label_create(cont);
    lv_label_set_text_static(label, event->name);
    lv_obj_set_width(label, LCD_H_RES*0.4);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP); 
    lv_obj_set_grid_cell(label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_END, 0, 1);

    //label for due date
    label = lv_label_create(cont);
    struct tm * timeinfo = localtime(&(event->start_time));
    char timestr[40];
    strftime(timestr, sizeof(timestr), "%D %r", timeinfo);
    lv_label_set_text(label, timestr);
    lv_obj_add_style(label, &style_text_muted, 0);
    lv_obj_set_grid_cell(label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 1, 1);
}


void updateTaskBuff(){
    taskBuffSize = RetrieveTasksSortedDB(taskBuffer, 4, taskCursor);
}

void updateEventBuff(){
    eventBuffSize = RetrieveEventsSortedDB(eventBuffer, 4, eventCursor);
}

void drawTasks(){
	if(lv_scr_act()==tile2){
        lv_obj_clean(tasklist);
        if(taskBuffSize == 0){
            lv_group_remove_obj(tasklist);
            return;
        }else{
            lv_group_add_obj(lv_group_get_default(), tasklist);
        }
        for(int i = 0; i < taskBuffSize; i++){
            if(taskBuffer[i].completion != MFD){
                create_task(&taskBuffer[i]);
            }
        }
    }
    
}

void drawEvents(){
	if(lv_scr_act()==tile2){
        lv_obj_clean(eventlist);
        if(eventBuffSize == 0){
            lv_group_remove_obj(eventlist);
            return;
        }else{
            lv_group_add_obj(lv_group_get_default(), eventlist);
        }
        time_t currTime = time(NULL);
        for(int i = 0; i < eventBuffSize; i++){
            create_event(&eventBuffer[i]);
        }
    }
}

static void tasks_left_cb(){
    if(taskCursor >= 4){
        taskCursor = taskCursor - 4;
        taskBuffSize = RetrieveTasksSortedDB(taskBuffer, 4, taskCursor);
        drawTasks();
    }
}

static void tasks_right_cb(){
    if(taskBuffSize == 4){
        taskCursor += 4;
        taskBuffSize = RetrieveTasksSortedDB(taskBuffer, 4, taskCursor);
        if(taskBuffSize != 0){
            drawTasks();
        }else{
            taskCursor -= 4;
        }
    }
}

static void events_left_cb(){
    if(eventCursor >= 4){
        eventCursor = eventCursor - 4;
        eventBuffSize = RetrieveEventsSortedDB(eventBuffer, 4, eventCursor);
        drawEvents();
    }
}

static void events_right_cb(){
    if(eventBuffSize == 4){
        eventCursor += 4;
        eventBuffSize = RetrieveEventsSortedDB(eventBuffer, 4, eventCursor);
        if(eventBuffSize != 0){
            drawEvents();
        }else{
            eventCursor -= 4;
        }
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
    
    //adds time to taskevent tile
    dateTime = lv_label_create(parent);
    lv_obj_set_style_text_font(dateTime, &lv_font_montserrat_18, 0);
    lv_label_set_text(dateTime, "loading...");
    lv_obj_align(dateTime, LV_ALIGN_TOP_LEFT, 5, 5);

    //arrow to indicate scroll up
    arrowUp = lv_label_create(parent);
    lv_label_set_text(arrowUp, LV_SYMBOL_UP);
    lv_obj_align(arrowUp, LV_ALIGN_TOP_RIGHT, -5, 5);
    
    //wifi symbol to taskevent tile
    wifiSymbol = lv_label_create(parent);
    lv_obj_set_style_text_font(wifiSymbol, &lv_font_montserrat_18, 0);
    lv_label_set_text(wifiSymbol, LV_SYMBOL_WIFI);
    lv_obj_align_to(wifiSymbol, arrowUp, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    lv_obj_add_flag(wifiSymbol, LV_OBJ_FLAG_HIDDEN);
    
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
    lv_obj_add_event_cb(button, events_left_cb, LV_EVENT_CLICKED, NULL);
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
    
    updateTaskBuff();
    updateEventBuff();
    drawTasks();
    drawEvents();
}


static const char * weekdays[7] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};

/*
 * creates callback function for habit buttons
 * takes in event which is the keypress
 */
static void buttonmatrix_cb6(lv_event_t * e){
    k = lv_event_get_key(e);
    time_t currtime = time(NULL);
    habit_t * habit = lv_event_get_user_data(e);
    lv_obj_t * obj = lv_event_get_target(e);
    
    if(k == LV_KEY_ENTER) {
        if (!lv_obj_has_state(obj, LV_STATE_CHECKED)){ //if toggled
            HabitAddEntry(habit->uuid, difftime(currtime, 1*86400));
        }else if(lv_obj_has_state(obj, LV_STATE_CHECKED)){ //if untoggled
            HabitRemoveEntry(habit->uuid, difftime(currtime, 1*86400));
        }
    }
}

static void buttonmatrix_cb5(lv_event_t * e){
    k = lv_event_get_key(e);
    time_t currtime = time(NULL);
    habit_t * habit = lv_event_get_user_data(e);
    lv_obj_t * obj = lv_event_get_target(e);
    
    if(k == LV_KEY_ENTER) {
        if (!lv_obj_has_state(obj, LV_STATE_CHECKED)){ //if toggled
            HabitAddEntry(habit->uuid, difftime(currtime, 2*86400));
        }else if(lv_obj_has_state(obj, LV_STATE_CHECKED)){ //if untoggled
            HabitRemoveEntry(habit->uuid, difftime(currtime, 2*86400));
        }
    }
}

static void buttonmatrix_cb4(lv_event_t * e){
    k = lv_event_get_key(e);
    time_t currtime = time(NULL);
    habit_t * habit = lv_event_get_user_data(e);
    lv_obj_t * obj = lv_event_get_target(e);
    
    if(k == LV_KEY_ENTER) {
        if (!lv_obj_has_state(obj, LV_STATE_CHECKED)){ //if toggled
            HabitAddEntry(habit->uuid, difftime(currtime, 3*86400));
        }else if(lv_obj_has_state(obj, LV_STATE_CHECKED)){ //if untoggled
            HabitRemoveEntry(habit->uuid, difftime(currtime, 3*86400));
        }
    }
}

static void buttonmatrix_cb3(lv_event_t * e){
    k = lv_event_get_key(e);
    time_t currtime = time(NULL);
    habit_t * habit = lv_event_get_user_data(e);
    lv_obj_t * obj = lv_event_get_target(e);
    
    if(k == LV_KEY_ENTER) {
        if (!lv_obj_has_state(obj, LV_STATE_CHECKED)){ //if toggled
            HabitAddEntry(habit->uuid, difftime(currtime, 4*86400));
        }else if(lv_obj_has_state(obj, LV_STATE_CHECKED)){ //if untoggled
            HabitRemoveEntry(habit->uuid, difftime(currtime, 4*86400));
        }
    }
}

static void buttonmatrix_cb2(lv_event_t * e){
    k = lv_event_get_key(e);
    time_t currtime = time(NULL);
    habit_t * habit = lv_event_get_user_data(e);
    lv_obj_t * obj = lv_event_get_target(e);
    
    if(k == LV_KEY_ENTER) {
        if (!lv_obj_has_state(obj, LV_STATE_CHECKED)){ //if toggled
            HabitAddEntry(habit->uuid, difftime(currtime, 5*86400));
        }else if(lv_obj_has_state(obj, LV_STATE_CHECKED)){ //if untoggled
            HabitRemoveEntry(habit->uuid, difftime(currtime, 5*86400));
        }
    }
}

static void buttonmatrix_cb1(lv_event_t * e){
    k = lv_event_get_key(e);
    time_t currtime = time(NULL);
    habit_t * habit = lv_event_get_user_data(e);
    lv_obj_t * obj = lv_event_get_target(e);
    
    if(k == LV_KEY_ENTER) {
        if (!lv_obj_has_state(obj, LV_STATE_CHECKED)){ //if toggled
            HabitAddEntry(habit->uuid, difftime(currtime, 6*86400));
        }else if(lv_obj_has_state(obj, LV_STATE_CHECKED)){ //if untoggled
            HabitRemoveEntry(habit->uuid, difftime(currtime, 6*86400));
        }
    }
}

static void buttonmatrix_cb0(lv_event_t * e){
    k = lv_event_get_key(e);
    time_t currtime = time(NULL);
    habit_t * habit = lv_event_get_user_data(e);
    lv_obj_t * obj = lv_event_get_target(e);
    
    if(k == LV_KEY_ENTER) {
        if (!lv_obj_has_state(obj, LV_STATE_CHECKED)){ //if toggled
            HabitAddEntry(habit->uuid, difftime(currtime, 7*86400));
        }else if(lv_obj_has_state(obj, LV_STATE_CHECKED)){ //if untoggled
            HabitRemoveEntry(habit->uuid, difftime(currtime, 7*86400));
        }
    }
}
/*
 * creates habit entry for habit list
 * takes in parent list and name of habit and the row number.
 */
void createHabit(habit_t * habit){
    //creates habit title name
    lv_obj_t * habits = lv_label_create(habitlist);
    lv_label_set_text_static(habits, habit->name);
    lv_obj_set_style_text_font(habits, &lv_font_montserrat_18, 0);
   
    //makes button map
    time_t currtime = time(NULL);
    struct tm * timeinfo = localtime(&currtime);
    
    //trying out button matrix
    buttonCont = lv_obj_create(habitlist);
    lv_gridnav_add(buttonCont, LV_GRIDNAV_CTRL_NONE);
    lv_obj_set_flex_flow(buttonCont, LV_FLEX_FLOW_ROW); //sets button to flex flow column form so it'll expand as needed
    //lv_obj_add_event_cb(buttonCont, buttonmatrix_cb, LV_EVENT_KEY, habit);
    lv_obj_set_style_bg_color(buttonCont, lv_palette_lighten(LV_PALETTE_BLUE, 4), LV_STATE_FOCUSED);
    lv_obj_set_style_pad_all(buttonCont, 5, LV_PART_MAIN);
    lv_group_add_obj(lv_group_get_default(), buttonCont);
    lv_obj_set_size(buttonCont, LCD_H_RES-50, LCD_V_RES/5.5);
    
    HabitRetrieveWeekCompletionDB(habit, currtime);
    for(int i = 0; i < 7; i++){
        button = lv_btn_create(buttonCont);
        lv_group_remove_obj(button);
        lv_obj_set_size(button, 50, 40);
        label = lv_label_create(button);
        lv_label_set_text(label, weekdays[(timeinfo->tm_wday + 1 + i) % 7]);
        lv_obj_center(label);
        lv_obj_add_flag(button, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_set_style_bg_color(button, lv_palette_lighten(LV_PALETTE_GREY, 1), LV_PART_MAIN);
        lv_obj_set_style_bg_color(button, lv_palette_lighten(LV_PALETTE_BLUE, 2), LV_STATE_CHECKED);
        if(habit->completed[6-i]==2){ //if user's indicated habit days, color of button should be changed
            lv_obj_set_style_bg_color(button, lv_palette_lighten(LV_PALETTE_RED, 1), LV_PART_MAIN);
            lv_obj_set_style_bg_color(button, lv_palette_lighten(LV_PALETTE_BLUE, 2), LV_STATE_CHECKED);
        }else if(habit->completed[6-i]==1){ //completed, set checked
            lv_obj_add_state(button, LV_STATE_CHECKED); 
        }
        switch(i){ //maybe this is a dumb way of doing it idk lol
        case 0:
            lv_obj_add_event_cb(button, buttonmatrix_cb0, LV_EVENT_KEY, habit);
            break;
        case 1:
            lv_obj_add_event_cb(button, buttonmatrix_cb1, LV_EVENT_KEY, habit);
            break;

        case 2:
            lv_obj_add_event_cb(button, buttonmatrix_cb2, LV_EVENT_KEY, habit);
            break;
        case 3:
            lv_obj_add_event_cb(button, buttonmatrix_cb3, LV_EVENT_KEY, habit);
            break;
        case 4:
            lv_obj_add_event_cb(button, buttonmatrix_cb4, LV_EVENT_KEY, habit);
            break;
        case 5:
            lv_obj_add_event_cb(button, buttonmatrix_cb5, LV_EVENT_KEY, habit);
            break;
        case 6:
            lv_obj_add_event_cb(button, buttonmatrix_cb6, LV_EVENT_KEY, habit);
            break;
        default:
            break;
        }
    }
}

void updateHabitBuff(){
    habitBuffSize = RetrieveHabitsDB(habitBuffer, 3, habitCursor);
}

void drawHabits(){
	if(lv_scr_act() == tile3){
        lv_obj_clean(habitlist);
        for(int i = 0; i < habitBuffSize; i++){
            createHabit(&habitBuffer[i]);
        }
    }
}
static void habits_left_cb(){
    if(habitCursor >= 3){
        habitCursor = habitCursor - 3;
        habitBuffSize = RetrieveHabitsDB(habitBuffer, 3, habitCursor);
        drawHabits();
    }
}

static void habits_right_cb(){
    if(habitBuffSize == 3){
        habitCursor += 3;
        habitBuffSize = RetrieveHabitsDB(habitBuffer, 3, habitCursor);
        if(habitBuffSize != 0){
            drawHabits();
        }else{
            habitCursor -= 3;
        }
    }
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
    lv_obj_align(button, LV_ALIGN_TOP_MID, -25, 5);
    lv_obj_add_event_cb(button, habits_left_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(button, 25, 25);
    label = lv_label_create(button);
    lv_label_set_text(label, LV_SYMBOL_LEFT);
    lv_obj_center(label);
    
    //right habit list arrows
    button = lv_btn_create(parent);
    lv_group_remove_obj(button);
    lv_obj_align(button, LV_ALIGN_TOP_MID, +25, 5);
    lv_obj_add_event_cb(button, habits_right_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(button, 25, 25);
    label = lv_label_create(button);
    lv_label_set_text(label, LV_SYMBOL_RIGHT);
    lv_obj_center(label);

    //arrow to indicate scroll up
    arrowUp = lv_label_create(parent);
    lv_label_set_text(arrowUp, LV_SYMBOL_UP);
    lv_obj_align(arrowUp, LV_ALIGN_TOP_RIGHT, -5, 5);
    
    //wifi symbol to habit tile
    wifiSymbol = lv_label_create(parent);
    lv_obj_set_style_text_font(wifiSymbol, &lv_font_montserrat_18, 0);
    lv_label_set_text(wifiSymbol, LV_SYMBOL_WIFI);
    lv_obj_align_to(wifiSymbol, arrowUp, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    lv_obj_add_flag(wifiSymbol, LV_OBJ_FLAG_HIDDEN);
    
    //creates a title for the habits tile
    lv_obj_t * habits = lv_label_create(parent);
    lv_label_set_text(habits, "Habit Progress");
    lv_obj_set_style_text_font(habits, &lv_font_montserrat_18, 0);
    lv_obj_align(habits, LV_ALIGN_TOP_LEFT, 5, 5);


    habitlist = lv_obj_create(parent);
    lv_obj_set_size(habitlist, lv_pct(100), lv_pct(90));
    lv_obj_align(habitlist, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_pad_top(habitlist, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(habitlist, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(habitlist, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(habitlist, LV_FLEX_FLOW_COLUMN); //sets button to flex flow column form so it'll expand as needed
    lv_obj_set_style_pad_row(habitlist, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(habitlist, LV_OPA_0, LV_PART_MAIN);
    
    updateHabitBuff();
    drawHabits();
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

/*
 * This file is created to be used in tandem with the LVGL sim, this way you can import the menus quickly and keep a separate main file with simulation specific functions.
 * Not intended to be used standalone, missing some library definitions and static variables. Intended to be imported as a .c file. Although maybe there's a way to do t
 */

static lv_obj_t *label = NULL;
static lv_style_t style_screen;

static lv_obj_t * tile1;
static lv_obj_t * tile2;
static lv_obj_t * tile3;

static lv_group_t * g1;

/* Text settings */
static lv_style_t style_text_muted;
static lv_style_t style_title;
static lv_style_t style_timer;
static const lv_font_t * font_large;
static const lv_font_t * font_giant;

/* Arrow up down symbols */
static lv_obj_t * arrowUp;
static lv_obj_t * arrowDown;
static lv_obj_t * arrowLeft;
static lv_obj_t * arrowRight;
static lv_obj_t * button;
static lv_obj_t * label;
static lv_obj_t * dateTime;
static lv_obj_t * child;
static lv_obj_t * obj;
static lv_obj_t * msgbox;
static lv_event_code_t code;
static uint32_t k;
const char * days[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sat"}; //for habits menu

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
        child = lv_obj_get_child(lv_event_get_target(e), NULL);
        lv_group_focus_obj(child);
    }else if(k == LV_KEY_RIGHT){
        child = lv_obj_get_child(lv_event_get_target(e), NULL);
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
 * I would like this to be configurable
 * but maybe later
 */
void initGroup(){
    g1 = lv_group_create();
    lv_group_set_default(g1);
}

/*
 * creates focus menu, takes in parent lv_obj
 */
void focusMenu_create(lv_obj_t * parent){
    //adding gridnav and focus colors to the taskevent menu
    lv_obj_set_style_bg_color(parent, lv_palette_lighten(LV_PALETTE_BLUE, 4), LV_STATE_FOCUSED);
    lv_gridnav_add(parent, LV_GRIDNAV_CTRL_VERTICAL_MOVE_ONLY);
    lv_group_add_obj(g1, parent);
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
    
    //TODO: needs time from RTC
    //adds time to focus tile
    dateTime = lv_label_create(parent);
    lv_label_set_text(dateTime, "4/11/2025 7:35 PM");
    lv_obj_set_style_text_font(dateTime, &lv_font_montserrat_18, 0);
    lv_obj_align(dateTime, LV_ALIGN_BOTTOM_LEFT, 5, -5);

    //arrow to indicate scroll down
    arrowDown = lv_label_create(parent);
    lv_label_set_text(arrowDown, LV_SYMBOL_DOWN);
    lv_obj_align(arrowDown, LV_ALIGN_BOTTOM_RIGHT, -5, -5);
}

static void msgbox_cb(lv_event_t * e){
    msgbox = lv_event_get_user_data(e);

    lv_msgbox_close(msgbox);
    lv_group_focus_freeze(g1, false);
}

//after the msg box is closed it always returns to the events list for some reason
//may figure out something for that maybe
static void task_desc_cb(lv_event_t * e){
    msgbox = lv_msgbox_create(NULL);
    //lv_msgbox_add_title(mbox, get_task_name);
    lv_msgbox_add_title(msgbox, "Task/Event title");
    //lv_msgbox_add_text(mbox, get_task_desc);
    lv_msgbox_add_text(msgbox, "Task/Event Description"); //also include due date

    button = lv_msgbox_add_footer_button(msgbox, "Exit");
    lv_obj_add_event_cb(button, msgbox_cb, LV_EVENT_CLICKED, msgbox);
    lv_group_focus_obj(button);
    lv_group_focus_freeze(g1, true);

    lv_obj_align(msgbox, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_style_bg_opa(lv_obj_get_parent(msgbox), LV_OPA_70, 0);
    lv_obj_set_style_bg_color(lv_obj_get_parent(msgbox), lv_palette_main(LV_PALETTE_GREY), 0);
}

/*
 * takes parent obj(usually an lv list), name of task, and due date of task
 * returns an lv_obj pointer to task list entry
 * probably needs to be fixed to be generic for tasks
 */
static lv_obj_t * create_task(lv_obj_t * parent, const char * name, const char * dueDate){
    //creates button for task using existing list button style
    lv_obj_t * cont = lv_obj_class_create_obj(&lv_list_button_class, parent);
    lv_obj_class_init_obj(cont);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN); //sets button to flex flow column form so it'll expand as needed
    lv_group_remove_obj(cont);   //Not needed, we use the gridnav instead
    lv_obj_add_event_cb(cont, task_desc_cb, LV_EVENT_CLICKED, name);
   
    //initalizes columns and rows of grid for the button so things are nicely aligned
    static int32_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static int32_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    
    //initializes grid
    lv_obj_set_grid_dsc_array(cont, grid_col_dsc, grid_row_dsc);
    lv_obj_set_style_pad_left(cont, 0, LV_PART_MAIN); //removes unnecessary left margin

    //label for task name
    lv_obj_t * label;
    label = lv_label_create(cont);
    lv_label_set_text_static(label, name);
    lv_obj_set_width(label, LCD_H_RES*0.4);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP); 
    lv_obj_set_grid_cell(label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_END, 0, 1);

    //label for due date
    label = lv_label_create(cont);
    lv_label_set_text_static(label, dueDate);
    lv_obj_add_style(label, &style_text_muted, 0);
    lv_obj_set_grid_cell(label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 1, 1);

    return cont;
}

void loadPrevTasks(){
    //TODO: load prev 4 tasks
}

void loadNextTasks(){
    //TODO: load next 4 tasks
}

void loadPrevEvents(){
    //TODO: load prev 4 events
}

void loadNextEvents(){
    //TODO: load next 4 events
}

static void tasks_left_cb(lv_event_t * e){
    code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        //TODO: load prev 4 tasks
    }
}

static void tasks_right_cb(lv_event_t * e){
    code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        //TODO: load next 4 tasks
    }
}

static void events_left_cb(lv_event_t * e){
    code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        //TODO: load prev 4 events
    }
}

static void events_right_cb(lv_event_t * e){
    code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        //TODO: load next 4 events
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
void taskEvent_create(lv_obj_t * parent){
    //adding gridnav and focus colors to the taskevent menu
    lv_obj_set_style_bg_color(parent, lv_palette_lighten(LV_PALETTE_BLUE, 4), LV_STATE_FOCUSED);
    lv_gridnav_add(parent, LV_GRIDNAV_CTRL_NONE);
    lv_group_add_obj(g1, parent);
    lv_group_focus_obj(parent);
    lv_obj_add_event_cb(parent, taskevent_cb, LV_EVENT_KEY, NULL);
    
    //TODO: needs time from RTC
    //adds time to taskevent tile
    dateTime = lv_label_create(parent);
    lv_label_set_text(dateTime, "4/11/2025 7:35 PM");
    lv_obj_set_style_text_font(dateTime, &lv_font_montserrat_18, 0);
    lv_obj_align(dateTime, LV_ALIGN_TOP_LEFT, 5, 5);

    //arrow to indicate scroll up
    arrowUp = lv_label_create(parent);
    lv_label_set_text(arrowUp, LV_SYMBOL_UP);
    lv_obj_align(arrowUp, LV_ALIGN_TOP_RIGHT, -5, 5);
    
    //make tasklist 
        
    //left task list arrows
    button = lv_btn_create(parent);
    //lv_group_add_obj(g1, button);
    lv_group_remove_obj(button);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, (LCD_H_RES/4)-25, 35);
    lv_obj_set_size(button, 25, 25);
    lv_obj_add_event_cb(button, tasks_left_cb, LV_EVENT_ALL, NULL);
    //lv_obj_add_event_cb(button, button_nav_cb, LV_EVENT_KEY, NULL);
    label = lv_label_create(button);
    lv_label_set_text(label, LV_SYMBOL_LEFT);
    lv_obj_center(label);
    
    //right task list arrows
    button = lv_btn_create(parent);
    //lv_group_add_obj(g1, button);
    lv_group_remove_obj(button);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, (LCD_H_RES/4)+25, 35);
    lv_obj_set_size(button, 25, 25);
    lv_obj_add_event_cb(button, tasks_right_cb, LV_EVENT_ALL, NULL);
    //lv_obj_add_event_cb(button, button_nav_cb, LV_EVENT_KEY, NULL);
    label = lv_label_create(button);
    lv_label_set_text(label, LV_SYMBOL_RIGHT);
    lv_obj_center(label);

    //left event list arrows
    button = lv_btn_create(parent);
    //lv_group_add_obj(g1, button);
    lv_group_remove_obj(button);
    lv_obj_align(button, LV_ALIGN_TOP_RIGHT, -((LCD_H_RES/4)+25), 35);
    lv_obj_add_event_cb(button, events_left_cb, LV_EVENT_ALL, NULL);//probably need to be updated for each button?
    //lv_obj_add_event_cb(button, button_nav_cb, LV_EVENT_KEY, NULL);
    lv_obj_set_size(button, 25, 25);
    label = lv_label_create(button);
    lv_label_set_text(label, LV_SYMBOL_LEFT);
    lv_obj_center(label);
    
    //right event list arrows
    button = lv_btn_create(parent);
    //lv_group_add_obj(g1, button);
    lv_group_remove_obj(button);
    lv_obj_align(button, LV_ALIGN_TOP_RIGHT, -((LCD_H_RES/4)-25), 35);
    //lv_obj_add_event_cb(button, events_right_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(button, button_nav_cb, LV_EVENT_KEY, NULL);
    lv_obj_set_size(button, 25, 25);
    label = lv_label_create(button);
    lv_label_set_text(label, LV_SYMBOL_RIGHT);
    lv_obj_center(label);

    //create lv list obj
    lv_obj_t * tasklist = lv_list_create(parent);
    lv_group_add_obj(g1, tasklist);
    lv_gridnav_add(tasklist, LV_GRIDNAV_CTRL_NONE);
    lv_obj_set_size(tasklist, lv_pct(49), lv_pct(73));
    lv_obj_align(tasklist, LV_ALIGN_TOP_LEFT, 3, 60);
    lv_obj_set_style_pad_all(tasklist, 0, LV_PART_MAIN);
   
    //dummy tasks
    //loadNextTasks();
    create_task(tasklist, "Capstone Project", "3/25/2025");
    create_task(tasklist, "Figure out Prototype", "3/29/2025");
    create_task(tasklist, "Learn PCB Design", "3/30/2025");
    create_task(tasklist, "Learn Computer Aided Design", "4/1/2025");
    
    //create a title for tasks
    lv_obj_t * taskTitle = lv_label_create(parent);
    lv_label_set_text(taskTitle, "Tasks");
    lv_obj_align(taskTitle, LV_ALIGN_BOTTOM_LEFT, 5, -5);
    
    //create event list
    //create lv list obj
    lv_obj_t * eventlist = lv_list_create(parent);
    lv_group_add_obj(g1, eventlist);
    lv_gridnav_add(eventlist, LV_GRIDNAV_CTRL_NONE);
    lv_obj_set_size(eventlist, lv_pct(49), lv_pct(73));
    lv_obj_align(eventlist, LV_ALIGN_TOP_LEFT, (LCD_H_RES/2), 60);
    lv_obj_set_style_pad_all(eventlist, 0, LV_PART_MAIN);
   
    //dummy events
    //TODO: make them iterate through the database to display
    //loadNextEvents();
    create_task(eventlist, "Capstone Meeting", "3/21/2025 3:00PM");
    create_task(eventlist, "ECE171 Class", "3/29/2025 2:00PM");
    create_task(eventlist, "CSE121 Class", "3/30/2025 5:00PM");
    
     //create a title for events
    lv_obj_t * eventTitle = lv_label_create(parent);
    lv_label_set_text(eventTitle, "Events");
    lv_obj_align(eventTitle, LV_ALIGN_BOTTOM_LEFT, (LCD_H_RES/2)+3, -5);

    //arrow to indicate scroll down
    arrowDown = lv_label_create(parent);
    lv_label_set_text(arrowDown, LV_SYMBOL_DOWN);
    lv_obj_align(arrowDown, LV_ALIGN_BOTTOM_RIGHT, -5, -5);
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
void createHabit(lv_obj_t * parent, const char * name, uint8_t row){
    //creates habit title name
    lv_obj_t * habits = lv_label_create(parent);
    lv_label_set_text(habits, name);
    lv_obj_set_style_text_font(habits, &lv_font_montserrat_18, 0);
    lv_obj_set_pos(habits, 15, 25+(row*100));
    
    //trying out button matrix
    lv_obj_t * buttons = lv_btnmatrix_create(parent);
    lv_gridnav_add(buttons, LV_GRIDNAV_CTRL_NONE);
    lv_obj_add_event_cb(buttons, buttonmatrix_cb, LV_EVENT_KEY, NULL);//probably need to be updated for each button?
    lv_obj_set_style_bg_color(parent, lv_palette_lighten(LV_PALETTE_BLUE, 4), LV_STATE_FOCUSED);
    lv_obj_set_style_pad_all(buttons, 5, LV_PART_MAIN);
    lv_group_add_obj(g1, buttons);
    lv_btnmatrix_set_map(buttons, btnm_map);
    lv_obj_set_pos(buttons, 15, 50+(row*100));
    lv_obj_set_size(buttons, LCD_H_RES-50, LCD_V_RES/5.5);
    for(uint8_t i = 0; i < 7; i++){
        lv_btnmatrix_set_btn_ctrl(buttons, i, LV_BTNMATRIX_CTRL_CHECKABLE);
    }
}

/*
 * creates habit menu, takes in parent lv obj
 */
void habitMenu_create(lv_obj_t * parent){
    //and color when focused and add gridnav 
    lv_obj_set_style_bg_color(parent, lv_palette_lighten(LV_PALETTE_BLUE, 4), LV_STATE_FOCUSED);
    lv_gridnav_add(parent, LV_GRIDNAV_CTRL_NONE);
    lv_group_add_obj(g1, parent);
    lv_group_focus_obj(parent);
    lv_obj_add_event_cb(parent, habit_cb, LV_EVENT_KEY, NULL);

    //arrow to indicate scroll up
    arrowUp = lv_label_create(parent);
    lv_label_set_text(arrowUp, LV_SYMBOL_UP);
    lv_obj_align(arrowUp, LV_ALIGN_TOP_RIGHT, -5, 5);

    /*
    //refresh button
    //not sure if we're gonna need this, idk if the database will auto update and lvgl will update accordingly prob not. prob needs to refresh the buffer every now and again.
    lv_obj_t * refreshButton = lv_button_create(parent);
    refresh = lv_label_create(refreshButton);
    lv_label_set_text(refresh, LV_SYMBOL_REFRESH);
    */

    //creates a title for the habits tile
    lv_obj_t * habits = lv_label_create(parent);
    lv_label_set_text(habits, "Habit Progress");
    lv_obj_set_style_text_font(habits, &lv_font_montserrat_18, 0);
    lv_obj_align(habits, LV_ALIGN_TOP_LEFT, 5, 5);

    //dummy habits
    //TODO: get habits from database
    createHabit(parent, "Go to the Gym", 0);
    createHabit(parent, "Walk the dog", 1);
    createHabit(parent, "Journal", 2);
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


/* Text settings */
static lv_style_t style_text_muted;
static lv_style_t style_title;
static lv_style_t style_timer;
static const lv_font_t * font_large;
static const lv_font_t * font_giant;

/* Arrow up down symbols */
static lv_obj_t * arrowUp;
static lv_obj_t * arrowDown;
const char * days[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sat"}; //for habits menu

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

/*
 * creates focus menu, takes in parent lv_obj
 */
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

    //arrow to indicate scroll down
    arrowDown = lv_label_create(parent);
    lv_label_set_text(arrowDown, LV_SYMBOL_DOWN);
    lv_obj_align(arrowDown, LV_ALIGN_BOTTOM_RIGHT, -5, -5);
}

/*
 * takes parent obj(usually an lv list), name of task, and due date of task
 * returns an lv_obj pointer to task list entry
 */
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

/*
 * creates menu and lists for tasklist and event list
 * takes in parent lv_obj
 */
void taskEvent_create(lv_obj_t * parent){
    //TODO: needs time from RTC
    lv_obj_t * dateTime = lv_label_create(parent);
    lv_label_set_text(dateTime, "4/11/2025 7:35 PM");
    lv_obj_set_style_text_font(dateTime, &lv_font_montserrat_18, 0);
    lv_obj_align(dateTime, LV_ALIGN_TOP_LEFT, 5, 5);

    //arrow to indicate scroll up
    arrowUp = lv_label_create(parent);
    lv_label_set_text(arrowUp, LV_SYMBOL_UP);
    lv_obj_align(arrowUp, LV_ALIGN_TOP_RIGHT, -5, 5);
    
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
    
    //arrow to indicate scroll down
    arrowDown = lv_label_create(parent);
    lv_label_set_text(arrowDown, LV_SYMBOL_DOWN);
    lv_obj_align(arrowDown, LV_ALIGN_BOTTOM_RIGHT, -5, -5);
}

/*
 * creates habit entry for habit list
 * takes in parent list and name of habit
 */
void createHabit(lv_obj_t * parent, const char * name){
    lv_obj_t * habits = lv_label_create(parent);
    lv_label_set_text(habits, name);
    lv_obj_set_style_text_font(habits, &lv_font_montserrat_18, 0);

    static int32_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    static int32_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

    lv_obj_t * week = lv_obj_create(parent);
    lv_obj_set_grid_dsc_array(week, grid_col_dsc, grid_row_dsc);
    lv_obj_set_size(week, LCD_H_RES-20, 55);
    lv_obj_set_style_pad_all(week, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(week, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(week, 0, LV_PART_MAIN);
    lv_obj_set_flex_align(week, LV_FLEX_ALIGN_CENTER, 0, 0);

    for(int i = 0; i < 7; i++) {
        lv_obj_t * obj = lv_button_create(week);
        lv_obj_set_size(obj, 50, 50);
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, i, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_set_style_bg_color(obj, lv_palette_main(LV_PALETTE_RED), 0);
        lv_obj_set_style_bg_color(obj, lv_palette_main(LV_PALETTE_BLUE), LV_STATE_CHECKED);
        //lv_group_remove_obj(obj);   //Not needed, we use the gridnav instead

        lv_obj_t * label = lv_label_create(obj);
        lv_label_set_text_fmt(label, "%s", days[i]);
        lv_obj_center(label);
    }
}

/*
 * creates habit menu, takes in parent lv obj
 */
void habitMenu_create(lv_obj_t * parent){
    //arrow to indicate scroll up
    arrowUp = lv_label_create(parent);
    lv_label_set_text(arrowUp, LV_SYMBOL_UP);
    lv_obj_align(arrowUp, LV_ALIGN_TOP_RIGHT, -5, 5);

    lv_obj_t * habits = lv_label_create(parent);
    lv_label_set_text(habits, "Habit Progress");
    lv_obj_set_style_text_font(habits, &lv_font_montserrat_18, 0);
    lv_obj_align(habits, LV_ALIGN_TOP_LEFT, 5, 5);
    
    lv_obj_t * habitList = lv_obj_create(parent);
    lv_obj_set_size(habitList, LCD_H_RES-20, LV_PCT(89));
    lv_obj_align(habitList, LV_ALIGN_CENTER, 0 , 17);
    lv_obj_set_flex_flow(habitList, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(habitList, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(habitList, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(habitList, 0, LV_PART_MAIN);
    
    createHabit(habitList, "Go to the Gym");
    createHabit(habitList, "Walk the dog");
    createHabit(habitList, "Journal");
    createHabit(habitList, "Get to Work on Time");
}



#ifndef GUICONTENT_H
#define GUICONTENT_H

#include "defs.h"
#include "main.h"

ScreenState AddTaskScreen(WINDOW *menu_win, DoublyLinkedList *list, sqlite3 *db);

#endif
#ifndef GUITASK_H
#define GUITASK_H

#include "defs.h"
#include "main.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]);

#define UNFILLED_BOX " "
#define FILLED_BOX "x"

void FocusMenu(WINDOW *focus_win, Node *highlight);

ScreenState TaskScreen(WINDOW *menu_win, sqlite3 *db, DoublyLinkedList *list, Node **highlight, size_t *n_tasks, size_t *list_offset);

#endif
#include "GUITask.h"
#include <stdlib.h>

// Free node data
void free_data(void *data)
{
    free(data);
}

// ----- Task List -----

/*
red - past due
yellow - day of
magenta - 2 days of
cyan - 3 days of
blue - 5 days of
green - 7 days of
white - more than a week
*/
int DeadLineColor(time_t dueDate)
{
    time_t currTime = time(NULL);

    if (currTime > dueDate)
    {
        return TEXT_RED;
    }

    time_t timeLeft = dueDate - currTime;

    if (timeLeft < 86400)
    {
        return TEXT_YELLOW;
    }
    else if (timeLeft < 172800)
    {
        return TEXT_MAGENTA;
    }
    else if (timeLeft < 259200)
    {
        return TEXT_CYAN;
    }
    else if (timeLeft < 432000)
    {
        return TEXT_BLUE;
    }
    else if (timeLeft < 604800)
    {
        return TEXT_GREEN;
    }
    else
    {
        return TEXT_WHITE;
    }
}

void FocusMenu(WINDOW *focus_win, Node *entry)
{
    if (!entry)
        return;

    Task *data = entry->data;

    int color = PriorityColor(data->priority);
    wattron(focus_win, COLOR_PAIR(color));
    mvwprintw(focus_win, 0, 0, "%s", data->name);
    wattroff(focus_win, COLOR_PAIR(color));

    // Convert timestamp to human-readable date
    char date_str[30]; // Buffer for the date string
    struct tm *hardtm;
    hardtm = localtime(&data->date);
    int hdColor = DeadLineColor(data->date);
    strftime(date_str, 30, "%Y-%m-%d %H:%M", hardtm); // Format as YYYY-MM-DD HH:MM
    mvwprintw(focus_win, 1, 0, "DLn: ");
    wattron(focus_win, COLOR_PAIR(hdColor));
    mvwprintw(focus_win, 1, 5, "%s", date_str);
    wattroff(focus_win, COLOR_PAIR(hdColor));

    if (data->sdate != -1)
    {
        char sdate_str[30]; // Buffer for the date string
        struct tm *softtm;
        softtm = localtime(&data->sdate);
        int sColor = DeadLineColor(data->sdate);
        strftime(sdate_str, 30, "%Y-%m-%d %H:%M", softtm);
        wattron(focus_win, COLOR_PAIR(sColor));
        mvwprintw(focus_win, 2, 0, "Aim: %s", sdate_str);
        wattroff(focus_win, COLOR_PAIR(sColor));
    }

    mvwprintw(focus_win, 1, 24, "Prio:");
    wattron(focus_win, COLOR_PAIR(color));
    mvwprintw(focus_win, 1, 29, "%u", data->priority);
    wattroff(focus_win, COLOR_PAIR(color));

    WordWrap(focus_win, data->desc, DESC_SIZE, 4, 0, FOCUS_WIDTH);
}

void PrintMenu(WINDOW *menu_win, DoublyLinkedList *list, Node *highlight, size_t offset)
{
    if (!highlight)
    {
        // LOG_INFO("Menu is empty");
        return;
    }

    int y, i;
    y = 1; // Start y position

    // Retrieve data
    Node *entry = list->head;

    // Offset
    for (int i = 0; i < offset; i++)
    {
        // Move to next entry
        entry = entry->next;
    }

    while (entry)
    {
        Task *data = entry->data;
        Task *selectedData = highlight->data;

        if (selectedData->id == data->id)
        {
            wattron(menu_win, A_REVERSE);
        }

        mvwprintw(menu_win, y, 1, "[");
        mvwprintw(menu_win, y, 2, "%s", data->status ? FILLED_BOX : UNFILLED_BOX);
        mvwprintw(menu_win, y, 3, "]");

        if (selectedData->id == data->id)
        {
            wattroff(menu_win, A_REVERSE);
        }

        // Print name
        int color = PriorityColor(data->priority);
        wattron(menu_win, COLOR_PAIR(color));
        mvwprintw(menu_win, y, 5, "%s", data->name);
        wattroff(menu_win, COLOR_PAIR(color));

        // Write due date (perferring soft deadline)
        time_t now = time(NULL);

        char date_str[30]; // Buffer for the date string
        struct tm *tm_info;
        int dateColor;
        if (data->sdate != -1 && now < data->sdate)
        {
            dateColor = DeadLineColor(data->sdate);
            tm_info = localtime(&data->sdate);
            strftime(date_str, 30, "%Y-%m-%d %H:%M", tm_info); // Format as YYYY-MM-DD HH:MM
            mvwprintw(menu_win, y, 38, " Aim: ");
            wattron(menu_win, COLOR_PAIR(dateColor));
            mvwprintw(menu_win, y, 44, "%s", date_str);
            wattroff(menu_win, COLOR_PAIR(dateColor));
        }
        else
        {
            dateColor = DeadLineColor(data->date);
            tm_info = localtime(&data->date);
            strftime(date_str, 30, "%Y-%m-%d %H:%M", tm_info); // Format as YYYY-MM-DD HH:MM
            mvwprintw(menu_win, y, 38, " DLn: ");
            wattron(menu_win, COLOR_PAIR(dateColor));
            mvwprintw(menu_win, y, 44, "%s", date_str);
            wattroff(menu_win, COLOR_PAIR(dateColor));
        }

        y += 2;

        // Move to next entry
        entry = entry->next;
    }
    wrefresh(menu_win);
}

ScreenState TaskScreen(WINDOW *menu_win, sqlite3 *db, DoublyLinkedList *list, Node **highlight, size_t *n_tasks, size_t *list_offset)
{
    int c;
    PrintMenu(menu_win, list, *highlight, *list_offset);
    c = wgetch(menu_win);

    switch (c)
    {
    case KEY_UP:
        if (!(*highlight))
            break;
        if ((*highlight)->prev == NULL)
            *highlight = list->tail;
        else
            *highlight = (*highlight)->prev;
        break;
    case KEY_DOWN:
        if (!(*highlight))
            break;
        if ((*highlight)->next == NULL)
            *highlight = list->head;
        else
            *highlight = (*highlight)->next;
        break;
    case KEY_NPAGE:
        if (*list_offset < *n_tasks - 1)
            (*list_offset)++;
        break;
    case KEY_PPAGE:
        if (*list_offset > 0)
            (*list_offset)--;
        break;
    case 'a': // Add tasks
        return ADD_TASK_SCREEN;
        break;
    case 27: // Exit (Esc key)
        return EXIT_APP;
        break;
    case 10: // Enter key
        // Toggle the status of the highlighted choice
        if (*highlight)
        {
            Task *selected_data = (*highlight)->data;
            selected_data->status = !selected_data->status;
            CompleteEntry(db, selected_data->id);
            SortList(&list);
        }
        break;
    case 'd': // Delete
        if (*highlight)
        {
            Task *selected_data = (*highlight)->data;

            // TODO: Proper UI for confirmation prompt
            LOG_INFO("Delete %s? (Y/n)", selected_data->name);
            char confirm = wgetch(menu_win);

            if (confirm == 'Y')
            {
                // Open data to identify data to remove
                LOG_INFO("Deleting %s...", selected_data->name);
                RemoveEntry(db, selected_data->id);

                Node *rm = *highlight;
                if (list->size == 1)
                    *highlight = NULL;
                else if ((*highlight)->prev == NULL)
                    *highlight = list->tail;
                else
                    *highlight = (*highlight)->prev;
                delete_node(list, rm, free_data);
            }
            else
            {
                LOG_INFO("Delete operation cancelled");
            }
        }
        else
        {
            LOG_WARNING("Cannot delete entry: No object selected");
        }
        break;
    }
    PrintMenu(menu_win, list, *highlight, *list_offset);
    return TASK_SCREEN;
}

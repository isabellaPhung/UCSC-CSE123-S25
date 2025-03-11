#include "main.h"
#include "GUITask.h"
#include "GUIContent.h"

int PriorityColor(int priority)
{
    // Set color based on priority
    int color;
    switch (priority)
    {
    case 0:
    case 1:
        color = 2;
        break;
    case 2:
    case 3:
        color = 4;
        break;
    case 4:
    case 5:
        color = 5;
        break;
    case 6:
    case 7:
        color = 3;
        break;
    case 8:
    case 9:
        color = 1;
        break;
    default:
        color = 7;
        break;
    }

    return color;
}

void WordWrap(WINDOW *focus_win, const char *buffer, size_t buffer_size, int y, int x, int width)
{
    if (buffer == NULL)
        return;
    if (strlen(buffer) > buffer_size)
    {
        LOG_ERROR("main::WordWrap: Buffer exceeds buffer size");
        return;
    }

    size_t w = 0;                  // Word iterator
    char smartdesc[buffer_size * 2]; // Buffer to hold formatted description
    size_t smartdesc_pos = 0;      // Current position in smartdesc
    smartdesc[0] = '\0';           // Initialize smartdesc

    char descword[buffer_size]; // Buffer to hold current word
    size_t line_length = 0;   // Current length of the line

    for (size_t i = 0; i < buffer_size && buffer[i] != '\0'; i++)
    {
        char c = buffer[i]; // Get char
        if (c == ' ' || c == '\n')
        {
            descword[w] = '\0'; // Terminate current word

            // Check if the word fits in the current line
            if (line_length + w + 1 > width)
            {
                // If not, insert a newline
                smartdesc[smartdesc_pos++] = '\n';
                line_length = 0;
            }

            // Add the word to smartdesc
            if (line_length > 0)
            {
                smartdesc[smartdesc_pos++] = ' '; // Add a space before the word if it's not the first word in the line
                line_length++;
            }

            strncpy(smartdesc + smartdesc_pos, descword, w);
            smartdesc_pos += w;
            line_length += w;

            // Reset word buffer
            w = 0;

            if (c == '\n')
            {
                smartdesc[smartdesc_pos++] = '\n';
                line_length = 0;
            }
        }
        else
        {
            descword[w++] = c; // Add char to current word
        }
    }

    // Ensure the last word is added
    if (w > 0)
    {
        descword[w] = '\0'; // Terminate the last word

        if (w < width && line_length + w + 1 > width)
        {
            smartdesc[smartdesc_pos++] = '\n';
        }
        else if (line_length > 0)
        {
            smartdesc[smartdesc_pos++] = ' ';
        }

        strncpy(smartdesc + smartdesc_pos, descword, w);
        smartdesc_pos += w;
    }

    smartdesc[smartdesc_pos] = '\0'; // Terminate the smartdesc buffer

    // Print formatted text (if possible)
    if (strlen(smartdesc) < buffer_size)
    {
        // Print formatted text without ncurses wrapping
        int row = y, col = x;
        for (size_t i = 0; i < smartdesc_pos; i++)
        {
            if (col > width || smartdesc[i] == '\n')
            {
                row++;
                col = x;
            }
            else
            {
                mvwaddch(focus_win, row, col++, smartdesc[i]);
            }
        }
    }
    else
        mvwprintw(focus_win, y, x, "%s", smartdesc);
}

void HintBox()
{
    return;
}

int main()
{
    // Windows use snake case
    WINDOW *menu_win, *focus_win, *hint_box,
        *menu_box, *focus_box, *console_box;

    // ----- Init Window -----

    initscr();
    curs_set(0); // Hide cursor
    start_color();

    // Initialize color pairs for priorities
    init_pair(TEXT_WHITE, COLOR_WHITE, COLOR_BLACK);
    init_pair(TEXT_RED, COLOR_RED, COLOR_BLACK);
    init_pair(TEXT_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(TEXT_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(TEXT_BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(TEXT_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(TEXT_CYAN, COLOR_CYAN, COLOR_BLACK);

    clear();
    noecho();
    cbreak(); // Line buffering disabled. pass on everything

    focus_win = newwin(FOCUS_LINES, FOCUS_WIDTH, 1, 1);
    focus_box = newwin(FOCUS_LINES + 2, FOCUS_WIDTH + 2, 0, 0);
    menu_win = newwin(MENU_LINES, MENU_WIDTH, 1, FOCUS_WIDTH + 4);
    menu_box = newwin(MENU_LINES + 2, MENU_WIDTH + 2, 0, FOCUS_WIDTH + 3);
    hint_box = newwin(1, CONSOLE_WIDTH + 2, FOCUS_LINES + 2, 0);

    console_win = newwin(CONSOLE_LINES, CONSOLE_WIDTH, FOCUS_LINES + 4, 1);
    console_box = newwin(CONSOLE_LINES + 2, CONSOLE_WIDTH + 2, FOCUS_LINES + 3, 0);
    console_enabled = true; // Enables console logging

    LOG_WARNING("This is a warning!");
    LOG_ERROR("This is an error!");

    keypad(menu_win, TRUE); // Enables arrow keys
    ScreenState current_screen = TASK_SCREEN;

    // ----- Create data -----

    sort_param = DATE;

    // Init SQL
    sqlite3 *db;
    InitSQL(&db);

    LOG_INFO("Finished creating database");

    // Init list
    DoublyLinkedList *list = create_list();
    size_t *n_tasks = &list->size;
    LOG_INFO("Finished creating DLL");

    size_t offset = 0;

    EatSQL(list, db);
    LOG_INFO("Grabbed %d tasks from SQL database!", *n_tasks);

    Node *highlight = list->head; // Currently selected node

    // ----- Output to terminal -----
    bool running = true;

    // Draw boxes
    wattron(focus_box, COLOR_PAIR(6));
    box(focus_box, 0, 0);
    wattroff(focus_box, COLOR_PAIR(6));
    wrefresh(focus_box);

    box(menu_box, 0, 0);
    wrefresh(menu_box);

    // Draw keyboard commands
    wattron(hint_box, A_REVERSE);
    wprintw(hint_box, "                       [A] Add Task [D] Delete [PgUp/PgDwn] Scroll Up/Down                       ");
    wattroff(hint_box, A_REVERSE);
    wrefresh(hint_box);

    if (console_enabled)
    {
        wattron(console_box, COLOR_PAIR(1));
        box(console_box, 0, 0);
        wattroff(console_box, COLOR_PAIR(1));
        wrefresh(console_box);
        LOG_INFO("Boxes drawn"); // Important to use or console will not display immediantly on boot
    }

    while (running)
    {
        // Clear the windows
        wclear(focus_win);
        wclear(menu_win);

        FocusMenu(focus_win, highlight);

        // Refresh windows
        wrefresh(focus_win);
        wrefresh(menu_win);

        // Find cursor
        if (!highlight && list->size > 0)
        {
            if (list->head)
            {
                highlight = list->head;
            }
            else
            {
                // If head is NULL list should be empty
                LOG_ERROR("main::main::List size error! Expected: 0 Got: %d", list->size);
            }
        }

        // Menu window state machine
        switch (current_screen)
        {
        case TASK_SCREEN:
            current_screen = TaskScreen(menu_win, db, list, &highlight, n_tasks, &offset);
            break;
        case ADD_TASK_SCREEN:
            current_screen = AddTaskScreen(menu_win, list, db);
            break;
        case EXIT_APP:
            running = false;
            break;
        }
    }

    LOG_INFO("Exiting...");
    clrtoeol();
    refresh();
    getch();
    endwin();
    return 0;
}

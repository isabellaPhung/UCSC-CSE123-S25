#ifndef SQL_H
#define SQL_H

#include <sqlite3.h>

#define UUID_SIZE 32
#define MAX_TASK_NAME_SIZE 128
#define MAX_TASK_DESC_SIZE 1024
#define MAX_EVENT_NAME_SIZE 128
#define MAX_EVENT_DESC_SIZE 1024

// Initializes the database and ensures the tasks table exists
int InitSQL(sqlite3 **db);

int CloseSQL(sqlite3 **db);

#endif  // SQL_H
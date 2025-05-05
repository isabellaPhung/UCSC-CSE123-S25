#ifndef SQL_H
#define SQL_H

#include <sqlite3.h>

// Library definitions
#include "../src/defs.h"

// Database tables
#include "../src/task.h"
#include "../src/event.h"

// Initializes the database and ensures the tasks table exists
int InitSQL(sqlite3 **db);

int CloseSQL(sqlite3 **db);

#endif // SQL_H
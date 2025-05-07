#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

// Library definitions
#include "defs.h"

// Database tables
#include "task.h"
#include "event.h"
#include "habit.h"

// Initializes the SPI protocol
esp_err_t init_shared_spi_bus();

// Initializes the database and ensures the tasks table exists
int InitSQL(sqlite3 **db);

int CloseSQL(sqlite3 **db);

#endif // DATABASE_H
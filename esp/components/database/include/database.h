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
//esp_err_t init_sqlite_memory(void);
esp_err_t MountSDCard();

// Database connection handling
sqlite3 *get_db_connection(void);
void release_db_connection(void);

// Initializes the database and ensures the tasks table exists
int InitSQL(sqlite3 **db);
int CloseSQL(sqlite3 **db);

// Saves json message to plaintext for later parsing
esp_err_t append_json_to_file(const char *filepath, cJSON *json);
esp_err_t append_payload_to_file(const char *payload);
esp_err_t ParseJSONFileToDatabase(const char *filepath);

#endif // DATABASE_H
#ifndef EVENT_H
#define EVENT_H

#include <sqlite3.h>
#include <time.h>

#include "defs.h"

#include "esp_err.h"

typedef struct
{
    char uuid[UUID_LENGTH];          // UUID string of entry
    char name[MAX_NAME_SIZE];        // Title of entry
    time_t start_time;               // Time the event starts from
    time_t duration;                 // Duration of event
    char description[MAX_DESC_SIZE]; // Verbose description of entry
} event_t;

esp_err_t RetrieveEventsSortedDB(sqlite3 *db, event_t *eventBuffer, int count, int offset);

esp_err_t ParseEventsJSON(sqlite3 *db, const char *json);

#endif
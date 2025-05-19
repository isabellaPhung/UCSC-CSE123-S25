#ifndef EVENT_H
#define EVENT_H

#include <sqlite3.h>
#include <time.h>
#include <cJSON.h>

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

/// @brief Retrieve events from database sorted from soonest to latest
/// @param dataase
/// @param eventBuffer Buffer the events are put into
/// @param count Number of events to put into the buffer
/// @param offset Offset of entries in the sorting function
int RetrieveEventsSortedDB(event_t *eventBuffer, int count, int offset);

/// @brief Parses entries from a JSON file to
/// @param db 
/// @param json 
/// @return 
esp_err_t ParseEventsJSON(cJSON *event);

/// @brief Deletes entry from database that shares the uuid
esp_err_t RemoveEventDB(const char *uuid);

// DEBUG FUNCTION
//esp_err_t TestEventFunctions(sqlite3 *db);

#endif
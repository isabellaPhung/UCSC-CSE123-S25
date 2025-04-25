#ifndef EVENT_H
#define EVENT_H

#include <sqlite3.h>
#include <time.h>

#include "SQL.h"

typedef struct
{
    sqlite3_int64 id;                      // id of entry within SQL
    char name[MAX_EVENT_NAME_SIZE];        // Title of entry
    time_t timeStart;                      // When the event begins (UTC)
    time_t timeDur;                        // Event duration (Seconds)
    char description[MAX_EVENT_DESC_SIZE]; // Verbose description of entry
} Event;

/**
 * @brief Adds entry to disk
 * @param db Database to be altered
 * @param ent Entry to be added to disk
 * @return SQLite Error
 */
int AddEventDB(sqlite3 *db, Event *ent);

/// @brief Deletes entry
/// @param db database to be altered
/// @param id entry id
/// @return SQLite Error
int RemoveEventDB(sqlite3 *db,
       sqlite3_int64 id);

// Retrieve task entries
int RetrieveEventDB(sqlite3 *db, sqlite3_int64 id, Event *ent);

#endif
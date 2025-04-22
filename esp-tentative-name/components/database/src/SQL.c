#include "Logger.h"
#include "SQL.h"

#include <stdio.h>
#include <sqlite3.h>

/// @brief Initializes the database and ensures the tasks table exists
/// @param db Database to be initialized. DATABASE MUST BE CLOSED MANUALLY.
/// @return SQLITE error code
int InitSQL(sqlite3 **db)
{

    int rc = sqlite3_open("calendar.db", db);
    if (rc)
    {
        LOG_ERROR("SQL::InitSQL: Can't open database: %s", sqlite3_errmsg(*db) ? sqlite3_errmsg(*db) : "No error code avalible");
        return rc;
    }

    // --- Task Table ---
    LOG_INFO("SQL::InitSQL: Creating Task Table...");
    // Ensures tasks table exists
    char *sql = "CREATE TABLE IF NOT EXISTS tasks ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT," // TODO: Convert to STRING for UUID key
                "name TEXT NOT NULL,"
                "datetime INTEGER NOT NULL,"    // Non-universal method of keeping time
                "priority INTEGER NOT NULL,"
                "completed INTEGER NOT NULL,"
                "description TEXT);";
    char *zErrMsg = 0;

    rc = sqlite3_exec(*db, sql, 0, 0, &zErrMsg); // Run SQL command

    // Error checking
    if (rc != SQLITE_OK)
    {
        LOG_ERROR("SQL::InitSQL: SQL error: %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return rc;
    }

    // --- Event Table ---
    LOG_INFO("SQL::InitSQL: Creating Event Table...");

    sql = "CREATE TABLE IF NOT EXISTS events ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "name TEXT NOT NULL,"
           "starttime INTEGER NOT NULL,"
           "duration INTEGER NOT NULL,"
           "description TEXT);";

    rc = sqlite3_exec(*db, sql, 0, 0, &zErrMsg); // Run SQL command
    // Error checking
    if (rc != SQLITE_OK)
    {
        LOG_ERROR("SQL::InitSQL: SQL error: %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return rc;
    }

    // --- TODO: Habit Table ---

    return SQLITE_OK;
}

int CloseSQL(sqlite3 **db)
{
    LOG_INFO("Closing database");
    int rc = sqlite3_close(*db);

    if (rc != SQLITE_OK)
    {
        LOG_ERROR("SQL::CloseSQL: Can't close database: $s", sqlite3_errmsg(*db) ? sqlite3_errmsg(*db) : "No error code avalible");
    }
}
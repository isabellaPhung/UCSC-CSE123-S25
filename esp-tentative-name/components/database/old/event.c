#include "event.h"

#include <esp_log.h>
#include <stddef.h>

static const char *TAG = "event";

int AddEventDB(sqlite3 *db, Event *ent)
{
	const char *sql;
	sqlite3_stmt *stmt;
	int rc;

	// Prepare the SQL statement based on the provided arguments
	sql = "INSERT INTO tasks (name, starttime, duration, description) "
		  "VALUES (?, ?, ?, ?);";

	// Prepare the SQL statement
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		ESP_LOGE(TAG, "Cannot prepare statement: %s", sqlite3_errmsg(db));
		return rc;
	}

	// Bind the values to the placeholders
	int index = 1;
	sqlite3_bind_text(stmt, index++, ent->name, -1, SQLITE_STATIC);

	sqlite3_bind_int64(stmt, index++, (sqlite3_int64)ent->timeStart);
	sqlite3_bind_int(stmt, index++, (int)ent->timeStart);
	sqlite3_bind_int(stmt, index++, (int)ent->timeDur);

	sqlite3_bind_text(stmt, index++, ent->description, -1, SQLITE_STATIC);

	// Execute the statement
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		ESP_LOGE(TAG, "Execution failed: %s", sqlite3_errmsg(db));
		sqlite3_finalize(stmt);
		return rc;
	}

	// Clean up
	sqlite3_finalize(stmt);

	ESP_LOGI(TAG, "SQL::AddEntry: Added Event: %s", ent->name);

	return SQLITE_OK;
}
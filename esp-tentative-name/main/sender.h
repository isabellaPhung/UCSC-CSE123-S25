/** Intermediary.h
 * This library is designed to send messages to the server from the user's input
 */
#ifndef INTERMEDIARY_H
#define INTERMEDIARY_H

#include "esp_err.h"
#include "database.h"

struct callback_data_t
{
    int expected;
    int cur_index;
    int update_ack;
    sqlite3 *db_ptr;
};

// Max amount of entries sent to the server on publish
#define MAX_ENTRIES 16

// Request folders
#define TASK_REQUESTS_DIR MOUNT_POINT "/task_status_requests/"
#define HABIT_REQUESTS_DIR MOUNT_POINT "/habit_status_requests/"

// ------------------------------------------ Tasks -----------------------------------------------

/// @brief NOT UpdateTaskStatusDB. Calls UpdateTaskStatusDB and saves a recipt to disk
/// @param db Database
/// @param uuid Unique ID
/// @param new_status Status to change. MFD deletes the task object from the database.
esp_err_t UpdateTaskStatus(sqlite3 *db, const char *uuid, TASK_STATUS new_status);

/// @brief Sends cached data to the server
/// @return ESP_OK if server acknowledged the request, clearing the cache
esp_err_t SyncTaskRequests(struct callback_data_t *cb_data);

#endif

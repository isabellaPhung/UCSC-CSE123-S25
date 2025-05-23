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
};

// Max amount of entries sent to the server on publish
#define MAX_ENTRIES 16

// Request folders
#define TASK_REQUESTS_DIR "/taskrequests"
#define HABIT_REQUESTS_DIR "/habitrequests"
#define EVENT_REQUESTS_DIR "/eventrequests"

// ------------------------------------------ Tasks -----------------------------------------------

/// @brief NOT UpdateTaskStatusDB. Calls UpdateTaskStatusDB and saves a recipt to disk
/// @param db Database
/// @param uuid Unique ID
/// @param new_status Status to change. MFD deletes the task object from the database.
esp_err_t UpdateTaskStatus(const char *uuid, TASK_STATUS new_status);

/// @brief Sends cached data to the server
/// @return ESP_OK if server acknowledged the request, clearing the cache
esp_err_t UploadTaskRequests(struct callback_data_t *cb_data, const char* device_id);

// ----------------------------------------- Habits -----------------------------------------------

esp_err_t HabitAddEntry(const char *habit_id, time_t datetime);
esp_err_t HabitRemoveEntry(const char *habit_id, time_t datetime);

esp_err_t UploadHabitRequests(struct callback_data_t *cb_data, const char* device_id);

// ------------------------------------------ Events ----------------------------------------------

esp_err_t RemoveEvent(const char *uuid);
esp_err_t UploadEventRequests(struct callback_data_t *cb_data, const char *device_id);

#endif

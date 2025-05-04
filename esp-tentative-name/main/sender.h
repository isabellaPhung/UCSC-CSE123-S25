/** Intermediary.h
 * This library is designed to send messages to the server from the user's input
 */
#ifndef INTERMEDIARY_H
#define INTERMEDIARY_H

#include "esp_err.h"
#include "database.h"

#define MAX_SEND_LENGTH 128 + UUID_LENGTH // Maximum length of JSON information that can be sent to the server at once

/// @brief Sends message to the server with a task's new status. Used for task completion or deletion
/// @param uuid Task ID
/// @param status The status the task has been changed to.
/// @return 0 on success, -1 on server failure
esp_err_t SendTaskStatus(const char *uuid, TASK_STATUS status);

// TODO: Send content of responses to the server, deletes content on success, terminates on failure.
int BufferToServer();

#endif

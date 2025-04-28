#include <sqlite3.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "../include/SQL.h"
#include "../include/task.h"

const char *json_data =
    "{\n"
    "  \"tasks\":[\n"
    "    {\n"
    "      \"id\":\"37a9b0d4-e523-43d5-b24a-38c5ce7f4bf5\",\n"
    "      \"name\":\"Walk the dog\",\n"
    "      \"description\":\"Walk the dog\",\n"
    "      \"completion\":\"incomplete\",\n"
    "      \"priority\":9,\n"
    "      \"duedate\":1744509600\n"
    "    },\n"
    "    {\n"
    "      \"id\":\"43b968d4-1d1c-4902-844a-91b75936b87d\",\n"
    "      \"name\":\"File taxes\",\n"
    "      \"description\":\"\",\n"
    "      \"completion\":\"incomplete\",\n"
    "      \"priority\":4,\n"
    "      \"duedate\":1744700340\n"
    "    },\n"
    "    {\n"
    "      \"id\":\"8dd144ea-ea15-4d5a-aa59-4223d4daa15d\",\n"
    "      \"name\":\"Work on the project\",\n"
    "      \"description\":\"See issues\",\n"
    "      \"completion\":\"incomplete\",\n"
    "      \"priority\":5,\n"
    "      \"duedate\":1744743000\n"
    "    }\n"
    "  ]\n"
    "}";

int rc; // Return code

void DB_task_demo(void *pvParameters)
{
    static const char *TAG = "SQL_DEMO";

    sqlite3 *db;

    ESP_LOGI(TAG, "SQL Database Tests For Schedule Companion\n");

    // ---------------------------------- Create Databse ----------------------------------
    ESP_LOGI(TAG, "Initializing database...");
    // Initialize the database and table
    if (InitSQL(&db) != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize database!");
        vTaskDelete(NULL);
    }
    ESP_LOGI(TAG, "Database initialized!\n");

    // ----------------------------------- Create Task ------------------------------------
    ESP_LOGI(TAG, "Adding a Task...");
    // Define a task
    Task newTask = {
        .time = time(NULL),
        .priority = 5,
        .completion = INCOMPLETE,
    };
    strncpy(newTask.uuid, "test-uuid-123", UUID_LENGTH);
    strncpy(newTask.name, "Test Task", MAX_TASK_NAME_SIZE);
    strncpy(newTask.description, "This is a test task.", MAX_TASK_DESC_SIZE);

    // Add task
    rc = AddTaskDB(db, &newTask);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to add task!");
        vTaskDelete(NULL);
    }
    ESP_LOGI(TAG, "Created Task!\n");

    // ------------------------------------ Retrieve Task ------------------------------------
    ESP_LOGI(TAG, "Retrieving Tasks...");
    Task tasks[3];
    RetrieveTasksSortedDB(db, tasks, 3);
    PrintTask(tasks[0]);
    ESP_LOGI(TAG, "Got Task (%s): %s\n", tasks[0].uuid, tasks[0].name);

    // ------------------------------------- Parse JSON ---------------------------------------
    ESP_LOGI(TAG, "Parsing JSON Into Dataset...");
    ESP_LOGI(TAG, "JSON Contents:\n%s\n", json_data);
    ParseTasksJSON(db, json_data);

    // ------------------------------------ Get Task List -------------------------------------
    RetrieveTasksSortedDB(db, tasks, 3);
    for (size_t i = 0; i < 3; i++)
    {
        PrintTask(tasks[i]);
    }

    // ------------------------------------ Complete Task -------------------------------------
    ESP_LOGI(TAG, "Completing task <%s>...", tasks[0].name);
    CompleteTaskDB(db, tasks[0].uuid);
    PrintTaskDB(db, tasks[0].uuid);
    // Note this only changes the status of the Database, tasks[0] is still marked as incomplete

    ESP_LOGI(TAG, "All tests passed!");
    // Close the database
    CloseSQL(&db);

    vTaskDelete(NULL); // Delete the task when it's finished
}
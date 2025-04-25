#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include "../src/SQL.h"
#include "../src/task.h"

#define RESET "\033[0m"
#define GREEN "\033[32m"
#define CYAN "\033[36m"

const char *json_data =
    "{\n"
    "  \"tasks\":[\n"
    "    {\n"
    "      \"id\":\"37a9b0d4-e523-43d5-b24a-38c5ce7f4bf5\",\n"
    "      \"name\":\"Walk the dog\",\n"
    "      \"description\":\"Walk the dog\",\n"
    "      \"completion\":\"incomplete\",\n"
    "      \"priority\":\"9\",\n"
    "      \"duedate\":1744509600\n"
    "    },\n"
    "    {\n"
    "      \"id\":\"43b968d4-1d1c-4902-844a-91b75936b87d\",\n"
    "      \"name\":\"File taxes\",\n"
    "      \"description\":\"\",\n"
    "      \"completion\":\"incomplete\",\n"
    "      \"priority\":\"4\",\n"
    "      \"duedate\":1744700340\n"
    "    },\n"
    "    {\n"
    "      \"id\":\"8dd144ea-ea15-4d5a-aa59-4223d4daa15d\",\n"
    "      \"name\":\"Work on the project\",\n"
    "      \"description\":\"See issues\",\n"
    "      \"completion\":\"incomplete\",\n"
    "      \"priority\":\"5\",\n"
    "      \"duedate\":1744743000\n"
    "    }\n"
    "  ]\n"
    "}";

// Quick script for making test descriptions distinct
void LOG_TEST(const char *format, ...)
{
    char buffer[256]; // Don't write super long error messages please
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    printf(CYAN ">>> %s" RESET, buffer);
}

int rc; // Return code

int main()
{
    sqlite3 *db;
    sqlite3_int64 entry_id; // Stores related ids

    printf("SQL Database Tests For Schedule Companion\n\n");

    // ---------------------------------- Create Databse ----------------------------------
    LOG_TEST("Initializing database...\n");
    // Initialize the database and table
    if (InitSQL(&db) != SQLITE_OK)
    {
        return 1;
    }
    LOG_TEST("Database initialized!\n\n");

    // ----------------------------------- Create Task ------------------------------------
    LOG_TEST("Adding a Task...\n");
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
        fprintf(stderr, "Failed to add task.\n");
    }
    LOG_TEST("Created Task!\n\n");

    // ------------------------------------ Retrieve Task ------------------------------------
    LOG_TEST("Retrieving Tasks...\n");
    Task tasks[3];
    RetrieveTasksSortedDB(db, tasks, 3);
    PrintTask(tasks[0]);
    LOG_TEST("Got Task (%s): %s\n\n", tasks[0].uuid, tasks[0].name);

    // ------------------------------------- Parse JSON ---------------------------------------
    LOG_TEST("Parsing JSON Into Dataset...\n");
    printf("JSON Contents:\n%s\n\n", json_data);
    ParseTasksJSON(db, json_data);
    LOG_TEST("Tasks should be in database now!\n\n");


    printf(GREEN "\nAll tests passed!\n" RESET);
    // Close the database
    sqlite3_close(db);
    return 0;
}
#include <stdio.h>
#include <sqlite3.h>
#include "../src/SQL.h"
#include "../src/task.h"

#define RESET "\033[0m"
#define GREEN "\033[32m"
#define CYAN "\033[36m"

// Quick script for making test descriptions distinct
void LOG_TEST(const char *format, ...)
{
	char buffer[256]; // Don't write super long error messages please
	va_list args;

	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	printf(CYAN ">>> %s\n" RESET, buffer);
}

int main()
{
	sqlite3 *db;
	sqlite3_int64 entry_id; // Stores related ids

	printf("SQL Database Tests For Schedule Companion\n\n");

	LOG_TEST("Initializing database...");
	// Initialize the database and table
	if (InitSQL(&db) != SQLITE_OK)
	{
		return 1;
	}
	LOG_TEST("Database initialized!\n");

	LOG_TEST("Create task with description:");
	// Add a task with a description
	if (AddTaskDB(db, &entry_id, "C moment", 1744414429, 1, 0, "Complete C project") != SQLITE_OK)
	{
		sqlite3_close(db);
		return 1;
	}
	LOG_TEST("Task created! ID: %lld", entry_id);
	// Print task 1 information
	if (PrintTaskDB(db, entry_id) != SQLITE_OK)
	{
		LOG_TEST("SQL Encountered an error: %s", sqlite3_errmsg(db));
		CloseSQL(&db);
		return 1;
	}

	LOG_TEST("Create task with NULL description:");
	// Add a task without a description
	if (AddTaskDB(db, &entry_id, "Stare at sun", 1744354429, 2, 0, NULL) != SQLITE_OK)
	{
		CloseSQL(&db);
		return 1;
	}
	LOG_TEST("Task created! ID: %lld\n", entry_id);
	// Print task 2 information
	if (PrintTaskDB(db, entry_id) != SQLITE_OK)
	{
		CloseSQL(&db);
		return 1;
	}

	LOG_TEST("Deleting task %lld...", entry_id);
	// Remove a task
	if (RemoveTaskDB(db, entry_id) != SQLITE_OK)
	{
		CloseSQL(&db);
		return 1;
	}
	LOG_TEST("Task %lld deleted!\n", entry_id);

	LOG_TEST("Adding another task after removal:");
	// Add a task without a description
	if (AddTaskDB(db, &entry_id, "Drink water", 1774314429, 9, 0, NULL) != SQLITE_OK)
	{
		CloseSQL(&db);
		return 1;
	}
	LOG_TEST("Task created! ID: %lld\n", entry_id);
	// Print task 2 information
	if (PrintTaskDB(db, entry_id) != SQLITE_OK)
	{
		CloseSQL(&db);
		return 1;
	}

	LOG_TEST("Filling database...");
	if (AddTaskDB(db, &entry_id, "Drink water", 1744362429, 9, 0, NULL) != SQLITE_OK)
	{
		CloseSQL(&db);
		return 1;
	}

	if (AddTaskDB(db, &entry_id, "Sleep", 1744364429, 6, 0, NULL) != SQLITE_OK)
	{
		CloseSQL(&db);
		return 1;
	}

	if (AddTaskDB(db, &entry_id, "Play video games", 1744364429, 8, 0, NULL) != SQLITE_OK)
	{
		CloseSQL(&db);
		return 1;
	}

	LOG_TEST("Ask for too many entries (10):");
	Task ents[10];
	int ret = RetrieveTasksSortedDB(db, ents, 10);

	if (ret < 0)
	{
		CloseSQL(&db);
		return 1;
	}

	for (int i = 0; i < ret; i++)
	{
		PrintTask(ents[i]);
	}

	LOG_TEST("Ask for less entries than in database (3):");
	ret = RetrieveTasksSortedDB(db, ents, 3);

	if (ret < 0)
	{
		CloseSQL(&db);
		return 1;
	}

	for (int i = 0; i < ret; i++)
	{
		PrintTask(ents[i]);
	}

	printf(GREEN "\nAll tests passed!\n" RESET);
	// Close the database
	sqlite3_close(db);
	return 0;
}
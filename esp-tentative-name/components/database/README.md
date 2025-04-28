# Database

The database component holds onto task and event entires, as well as habit lists.

## `SQL.h`

SQL.h library is an initializer script for the database.
- Initilaizes SQLite3 struct
- It should write empty databases to storage if not found

## `task.h`

Adds, retrieves, removes, or completes **task** entries

# Configuration

Currently the SD card uses the wire configuration indicated below:

SD card pin | SPI pin | ESP32-C3 and other chips |
------------|---------|--------------------------|
 D0         | MISO    | GPIO21                    |
 D3         | CS      | GPIO9                    |
 CLK        | SCK     | GPIO7                    |
 CMD        | MOSI    | GPIO20                    |

**IMPORTANT**: Every pin must have a 10k pull up resistor to 3.3V.

## Menuconfig

`Clear database on boot` deletes the database file when the device boots. Since each ID has to be unique as is part of the protocol this is necessary for our test scripts which repeadedly load the same information, but *devistating for further use in our prototype*.

`Test SD card on boot` does a test read and write from the SD card as a sanity check. It's pretty useless all things considered.

# SQLite3 ESP32
The database will rely on [SQLite3 for the esp32](https://github.com/nopnop2002/esp32-idf-sqlite3) for database management. It is implemented as a submodule, if you haven't used `--recurse-submodules` flag you can use the following bash commands to download it:

```bash
git submodule init
git submodule update
```

**IMPORTANT**: This directory attempts to use flags that do not work with the riscv compiler. Current fix is to go into the `sqlite3/components/esp32-idf-sqlite3` directory, and remove all references to the flag "-mlongcalls" in `component.mk` and `CMakeLists.txt` . Doing this will allow the project to build.

# Usage

This library will automatically move formatted JSON strings to the local SQL database. These strings *must* have the following syntax:
```json
{
    tasks:[
        {
            id: <UNIQUE string>,
            name: <string>,
            description: <string>,
            completion: <"complete"/"incomplete">,  // To be converted into an int later
            priority: <int>,
            duedate: <int>  // Time since epoch. TODO: Convert to ISO 8601 timestamp
        }
    ]
}
```

```c
// Create new database object
sqlite3 *db;
InitSQL(&db);

// Push contents of formatted json string to database
ParseTasksJSON(db, (const char*)json_data);

// Retrieve the 3 "top" tasks
Task tasks[3];
RetrieveTasksSortedDB(db, tasks, 3);
// Task contents:
Task *ent = tasks[0];
ESP_LOGI(TAG, "UUID: %s | Name: %s | Due: %lld | Priority: %d | Completed: %d | Description: %s",
             ent->uuid, ent->name, ent->time, ent->priority, ent->completion, ent->description);
// PrintTask(Task) is a helper function that can just do this

// Complete top task
CompleteTaskDB(db, (const char*)tasks[0].uuid);

// Remove last task
RemoveTaskDB(db, (const char*)tasks[2].uuid);

// Clean up
CloseSQL(&db);
```

# TODO
- [x] Task Table
- [ ] Event Table
- [ ] Habit Table
- [x] Read from JSON to SQLite
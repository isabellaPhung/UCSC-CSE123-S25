# Database

The database component holds onto task and event entires, as well as habit lists.

## `database.h`

database.h library is an initializer script for the database.
- Initilaizes SQLite3 struct
- It should write empty databases to storage if not found

# `task.h`

Adds, retrieves, removes, or completes **task** entries. These entries are defined as:

```c
typedef struct
{
    char uuid[UUID_LENGTH];          // UUID string of entry
    char name[MAX_NAME_SIZE];        // Title of entry
    time_t time;                     // Unix time (UTC)
    char priority;                   // Priority
    TASK_STATUS completion;          // Completion status of the entry
    char description[MAX_DESC_SIZE]; // Verbose description of entry
} task_t;
```

## Usage

This library will automatically move formatted JSON strings to the local SQL database. These strings *must* have the following syntax:

```json
{
    task:[
        {
            id: <UNIQUE string>,
            name: <string>,
            description: <string>,
            completion: <0 (incomplete), 1 (complete), or 2 (MFD)>,
            priority: <int>,
            duedate: <int>  // Time since epoch
        }
    ]
}
```


```c
#include "database.h"

// Create new database object
sqlite3 *db;
InitSQL(&db);

// Push contents of formatted json string to database
ParseTasksJSON(db, json_data);

// Retrieve the 3 "top" tasks with an offset of 0
Task tasks[3];
RetrieveTasksSortedDB(db, tasks, 3, 0);
// Task contents:
Task *ent = tasks[0];
ESP_LOGI(TAG, "UUID: %s | Name: %s | Due: %lld | Priority: %d | Completed: %d | Description: %s",
             ent->uuid, ent->name, ent->time, ent->priority, ent->completion, ent->description);
// PrintTask(Task) is a helper function that can just do this

// Complete top task
UpdateTaskStatusDB(db, tasks[0].uuid, COMPLETE);

// Remove last task
// IMPORTANT: Setting a task to be Marked For Deletion will straight up DELETE it from the database,
// NOT change the completion value in the object.
// The database will not allow any object Marked For Deletion to remain inside it
UpdateTaskStatusDB(db, tasks[2].uuid, MFD);

// Clean up
CloseSQL(&db);
```

# `event.h`

Events are similar to tasks yet hold duration instead of completion date, indicating time periods.

```c
typedef struct
{
    char uuid[UUID_LENGTH];          // UUID string of entry
    char name[MAX_NAME_SIZE];        // Title of entry
    time_t start_time;               // Time the event starts from
    time_t duration;                 // Duration of event
    char description[MAX_DESC_SIZE]; // Verbose description of entry
} event_t;
```

Similar to the task type, the event type accepts JSON and outputs to a `event_t` list. The following JSON schema is necessary to use `ParseEventsJSON`:

```json
{
    event:
        {
            id: <UNIQUE string>,
            name: <string>,
            description: <string>,
            starttime: <int>,      // Time since epoch
            duration: <int>        // Seconds
        }
}
```

# `habit.h`

Habits rely soley on the database for storage and usage. Habits are comprised of two parts, the habit type, and the habit entires. When the user completes the habit a new entry is stored to the database with the date and reference to habit type.

Habit types define two values:
- The name of the habit.
- The frequency at which the user will preform the action.

```json
{
    habit:
        {
            id: <UNIQUE string>,
            name: <string>,
            goal_flags: <int>
        }
}
```

`goal_flags` represents which day of the week the task *should* be done, which can be indicated on the GUI. 7 bits of this integer coorespond to days of the week:

| bit     | day       |
|---------|-----------|
| 0 (LSB) | Saturday  |
| 1       | Friday    |
| 2       | Thursday  |
| 3       | Wednesday |
| 4       | Tuesday   |
| 5       | Monday    |
| 6 (MSB) | Sunday    |

# Configuration

Currently the SD card uses the wire configuration indicated below:

SD card pin | SPI pin | ESP32-C3 and other chips |
------------|---------|--------------------------|
 D0         | MISO    | GPIO21                   |
 D3         | CS      | GPIO9                    |
 CLK        | SCK     | GPIO7                    |
 CMD        | MOSI    | GPIO20                   |

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

# TODO
- [x] Task Table
- [x] Event Table
- [ ] Habit Table
- [x] Read from JSON to SQLite
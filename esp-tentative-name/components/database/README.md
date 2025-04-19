# Database

The database component holds onto task and event entires, as well as habit lists.

## `SQL.h`

SQL.h library is an initializer script for the database.
- Initilaizes SQLite3 struct
- It should write empty databases to storage if not found

## `task.h`

Adds, retrieves, removes, or completes task entries

# Configuration

Currently the SD card uses the wire configuration indicated below:

SD card pin | SPI pin | ESP32-C3 and other chips |
------------|---------|--------------------------|
 D0         | MISO    | GPIO6                    |
 D3         | CS      | GPIO1                    |
 CLK        | SCK     | GPIO5                    |
 CMD        | MOSI    | GPIO4                    |

IMPORTANT: Every pin must have a 10k pull up resistor to 3.3V.

# SQLite3

Currently the database uses SQLite3. This can be installed using:

```shell
sudo apt update
sudo apt -y install libsqlite3-dev
```

# SQLite3 ESP32
The database will rely on [SQLite3 for the esp32](https://github.com/nopnop2002/esp32-idf-sqlite3) for database management.
- Note: This directory attempts to use flags that do not work with the riscv compiler. Current fix is to go into `build.ninja` and `compile_commands.json` and remove all references to the flag "-mlongcalls". Doing this will allow the project to build.
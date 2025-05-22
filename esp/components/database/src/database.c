#include "database.h"

#include <sqlite3.h>
#include <semaphore.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"

#include <unistd.h>
#include <errno.h>  // For errno
#include <string.h> // For strerror()

// Pin assignments
#define PIN_NUM_MISO CONFIG_SD_PIN_MISO
#define PIN_NUM_MOSI CONFIG_SD_PIN_MOSI
#define PIN_NUM_CLK CONFIG_SD_PIN_CLK
#define PIN_NUM_CS CONFIG_SD_PIN_CS

#define TEST_SD_CARD CONFIG_TEST_SD_CARD
#define CLEAN_DATABASE CONFIG_CLEAN_DATABASE

static bool sdcard_mounted = false;

/* Testing Scripts */
void DeleteDatabaseIfExists(char *database_path);
esp_err_t TestSDCard();

/*======================================================= Hardware Initialzation =======================================================*/

// Initialize SPI Protocol
esp_err_t init_shared_spi_bus()
{
    static bool spi_initialized = false;
    if (spi_initialized)
        return ESP_OK;

    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 1024, // Big enough for both SD and LCD, adjust if needed
    };
    esp_err_t ret = spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret == ESP_OK)
        spi_initialized = true;
    return ret;
}

/// @brief Mounts the SD Card if not already mounted
/// @return ESP_OK on success, ESP_FAIL otherwise
esp_err_t MountSDCard()
{
    const char *TAG = "database::MountSDCard";

    if (sdcard_mounted)
        return ESP_OK;

    esp_err_t ret;
    sdmmc_card_t *card;

    /* SPI initialization */
    ret = init_shared_spi_bus();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "SPI bus init failed");
        return ret;
    }

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .max_files = 2, // Maximum number of files that can be open at the same time
        .format_if_mount_failed = true,
        .allocation_unit_size = 512,
    };

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI_HOST; // Mak sure this matches shared bus

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to mount filesystem: %s", esp_err_to_name(ret));
        return ret;
    }

    sdcard_mounted = true;
    sdmmc_card_print_info(stdout, card);

    return ESP_OK;
}

/*======================================================= Database Initialzation =======================================================*/

/*
static uint8_t *sqlite_memory = NULL;
int sqlite_initialized = false;

// Creates a static heap for sqlite3 execution
esp_err_t init_sqlite_memory(void)
{
    const char *TAG = "database::init_sqlite_memory";

    if (sqlite_initialized)
    {
        ESP_LOGI(TAG, "SQLite already initialized");
        return ESP_OK;
    }

    // Check current free heap
    size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    ESP_LOGI(TAG, "Available heap before SQLite memory allocation: %u bytes", free_heap);

    if (free_heap < SQLITE_MEMORY_SIZE + 8192) // keep headroom
    {
        ESP_LOGW(TAG, "Not enough heap to safely allocate SQLite memory (%d requested)", SQLITE_MEMORY_SIZE);
        return ESP_ERR_NO_MEM;
    }

    sqlite_memory = heap_caps_malloc(SQLITE_MEMORY_SIZE, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    if (sqlite_memory == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate SQLite memory pool");
        return ESP_ERR_NO_MEM;
    }

    // Configure SQLite to use our memory pool - MUST BE DONE BEFORE ANY OTHER SQLITE CALLS
    int rc = sqlite3_config(SQLITE_CONFIG_HEAP, sqlite_memory, SQLITE_MEMORY_SIZE, 64); // 64-byte alignment
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "sqlite3_config failed with code %d: %s", rc, sqlite3_errstr(rc));
        heap_caps_free(sqlite_memory);
        sqlite_memory = NULL;
        return ESP_FAIL;
    }

    sqlite_initialized = true;
    ESP_LOGI(TAG, "Allocated %d bytes for SQLite memory pool and initialized SQLite", SQLITE_MEMORY_SIZE);
    return ESP_OK;
}
*/

/// @brief Initializes the database and ensures the tasks table exists
/// @param db Database to be initialized. DATABASE MUST BE CLOSED MANUALLY.
/// @return SQLITE error code
int InitSQL(sqlite3 **db)
{
    const char *TAG = "database::InitSQL";

    if (MountSDCard() != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to mount SD card.");
        return SQLITE_CANTOPEN;
    }

#ifdef CONFIG_TEST_SD_CARD
    if (TestSDCard() != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed SD card test.");
        return SQLITE_CANTOPEN;
    }
#endif

    // Build database path
    char db_path[32]; // increased buffer for safety
    snprintf(db_path, sizeof(db_path), MOUNT_POINT "/calendar.db");

#ifdef CONFIG_CLEAN_DATABASE
    DeleteDatabaseIfExists(db_path);
#endif

    ESP_LOGW(TAG, "Free heap before opening database: %lu bytes", esp_get_free_heap_size());

    // Initialize SQLite only after configuring memory
    int rc = sqlite3_initialize();
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "sqlite3_initialize failed with code %d: %s", rc, sqlite3_errstr(rc));
        return ESP_FAIL;
    }

    // Open SQLite database with NOMUTEX for reduced overhead
    rc = sqlite3_open_v2(db_path, db,
                         SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL);

    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Can't open database: %s", sqlite3_errmsg(*db));
        return rc;
    }

    {
        // Configure SQLite with optimized memory settings for ESP32
        char *pragma_cmds[] = {
            "PRAGMA page_size=512;",          // Smaller page size
            "PRAGMA cache_size=-20;",         // 40 pages (around 20KB)
            "PRAGMA temp_store=MEMORY;",      // Store temp tables in memory
            "PRAGMA journal_mode=MEMORY;",    // In-memory journal
            "PRAGMA synchronous=OFF;",        // Less safe but much faster
            "PRAGMA locking_mode=EXCLUSIVE;", // Better for single-connection usage
            NULL};

        for (int i = 0; pragma_cmds[i] != NULL; i++)
        {
            rc = sqlite3_exec(*db, pragma_cmds[i], NULL, NULL, NULL);
            if (rc != SQLITE_OK)
            {
                ESP_LOGW(TAG, "Failed to set pragma: %s", pragma_cmds[i]);
            }
        }
    }

    // ---------- Task Table ----------
    ESP_LOGI(TAG, "Creating Task Table...");
    const char *sql_task =
        "CREATE TABLE IF NOT EXISTS tasks ("
        "id TEXT PRIMARY KEY,"
        "name TEXT NOT NULL,"
        "datetime INTEGER NOT NULL,"
        "priority INTEGER NOT NULL,"
        "completed INTEGER NOT NULL,"
        "description TEXT);";

    rc = sqlite3_exec(*db, sql_task, NULL, NULL, NULL);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to create task table: %s", sqlite3_errmsg(*db));
        return rc;
    }

    // ---------- Event Table ----------
    ESP_LOGI(TAG, "Creating Event Table...");
    const char *sql_event =
        "CREATE TABLE IF NOT EXISTS events ("
        "id TEXT PRIMARY KEY,"
        "name TEXT NOT NULL,"
        "starttime INTEGER NOT NULL,"
        "duration INTEGER NOT NULL,"
        "description TEXT);";

    rc = sqlite3_exec(*db, sql_event, NULL, NULL, NULL);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to create event table: %s", sqlite3_errmsg(*db));
        return rc;
    }

    // ---------- Habit Tables ----------
    ESP_LOGI(TAG, "Creating Habit Tables...");
    const char *sql_habits =
        "CREATE TABLE IF NOT EXISTS habits ("
        "id TEXT PRIMARY KEY, "
        "name TEXT UNIQUE NOT NULL, "
        "day_goals INT NOT NULL);";

    const char *sql_entries =
        "CREATE TABLE IF NOT EXISTS habit_entries ("
        "habit_id TEXT NOT NULL, "
        "date TEXT NOT NULL, "
        "PRIMARY KEY (habit_id, date), "
        "FOREIGN KEY (habit_id) REFERENCES habits(id) ON DELETE CASCADE);";

    rc = sqlite3_exec(*db, sql_habits, NULL, NULL, NULL);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to create habits table: %s", sqlite3_errmsg(*db));
        return rc;
    }

    rc = sqlite3_exec(*db, sql_entries, NULL, NULL, NULL);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "Failed to create habit entries table: %s", sqlite3_errmsg(*db));
        return rc;
    }

    return SQLITE_OK;
}

int CloseSQL(sqlite3 **db)
{
    const char *TAG = "database::CloseSQL";

    ESP_LOGI(TAG, "Closing database");
    int rc = sqlite3_close(*db);

    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "database::CloseSQL: Can't close database: %s", sqlite3_errmsg(*db) ? sqlite3_errmsg(*db) : "No error code available");
        return rc;
    }

    return SQLITE_OK;
}

//*======================================================= Safe connection and Disconnection Logic =======================================================*/

static sqlite3 *shared_db = NULL;
static int connection_users = 0;
static SemaphoreHandle_t db_mutex = NULL;

sqlite3 *get_db_connection(void)
{
    if (db_mutex == NULL)
    {
        db_mutex = xSemaphoreCreateMutex();
    }

    xSemaphoreTake(db_mutex, portMAX_DELAY);

    if (shared_db == NULL)
    {
        InitSQL(&shared_db);
    }

    connection_users++;
    xSemaphoreGive(db_mutex);

    return shared_db;
}

void release_db_connection(void)
{
    if (db_mutex == NULL || shared_db == NULL)
    {
        return;
    }

    xSemaphoreTake(db_mutex, portMAX_DELAY);
    connection_users--;

    // Only close when no users left and low-memory conditions
    if (connection_users <= 0 && esp_get_free_heap_size() < 10000)
    {
        CloseSQL(&shared_db);
        shared_db = NULL;
        connection_users = 0;
    }

    xSemaphoreGive(db_mutex);
}

/*======================================================= Initialzation Debug Functions =======================================================*/

void DeleteDatabaseIfExists(char *database_path)
{
    const char *TAG = "database::DeleteDatabase";

    ESP_LOGW(TAG, "CLEAN_DATABASE Field is ON, DELETING DATABASE!");

    // Check if the file exists
    if (access(database_path, F_OK) == 0)
    {
        ESP_LOGI(TAG, "Database file exists. Attempting to delete...");
        if (unlink(database_path) == 0)
        {
            ESP_LOGI(TAG, "Database deleted successfully.");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to delete database: errno=%d (%s)", errno, strerror(errno));
        }
    }
    else
    {
        ESP_LOGW(TAG, "Database file does not exist. No need to delete. (errno=%d: %s)", errno, strerror(errno));
    }
}

esp_err_t TestSDCard()
{
    const char *TAG = "database::TestSDCard";

    ESP_LOGI(TAG, "Testing file system...");

    // Perform a simple test: open a file, write data, then close it
    FILE *f = fopen(MOUNT_POINT "/test.txt", "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open test file for writing");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Test file opened successfully, writing data...");
    fprintf(f, "SD card write successful!\n");

    // Move file pointer to the start and read to verify the contents
    fseek(f, 0, SEEK_SET);
    char line[128]; // Buffer to read lines
    while (fgets(line, sizeof(line), f))
    {
        ESP_LOGI(TAG, "Read line: %s", line); // Print each line to the console
    }

    fclose(f);
    ESP_LOGI(TAG, "Test file written successfully!");
    return ESP_OK;
}

/*======================================================= Database Tools =======================================================*/

esp_err_t append_payload_to_file(const char *payload)
{
    const char *TAG = "append_payload_to_file";

    if (payload == NULL)
    {
        ESP_LOGE(TAG, "Invalid payload");
        return ESP_ERR_INVALID_ARG;
    }

    // Open file for appending
    FILE *fp = fopen(MESSAGE_BUFFER_NAME, "a");
    if (fp == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file: %s", MESSAGE_BUFFER_NAME);
        return ESP_FAIL;
    }

    // Write the JSON string followed by newline
    fprintf(fp, "%s\n", payload);
    fclose(fp);

    ESP_LOGI(TAG, "Appended payload to file: %s", MESSAGE_BUFFER_NAME);
    return ESP_OK;
}

esp_err_t append_json_to_file(const char *filepath, cJSON *json)
{
    const char *TAG = "append_json_to_file";

    if (json == NULL)
    {
        ESP_LOGE(TAG, "Cannot write NULL cJSON object");
        return ESP_ERR_INVALID_ARG;
    }

    // Convert cJSON object to a string
    char *json_string = cJSON_PrintUnformatted(json); // Compact format
    if (json_string == NULL)
    {
        ESP_LOGE(TAG, "Failed to serialize cJSON object");
        return ESP_FAIL;
    }

    // Open file for appending
    FILE *fp = fopen(filepath, "a");
    if (fp == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file: %s", filepath);
        cJSON_free(json_string);
        return ESP_FAIL;
    }

    // Write the JSON string followed by newline
    fprintf(fp, "%s\n", json_string);
    fclose(fp);
    cJSON_free(json_string); // Free string after writing

    ESP_LOGI(TAG, "Appended JSON object to file: %s", filepath);
    return ESP_OK;
}

esp_err_t ParseJSONFileToDatabase(const char *filepath)
{
    const char *TAG = "ParseJSONFileToDatabase";
    FILE *fp = fopen(filepath, "r");

    if (!fp)
    {
        ESP_LOGE(TAG, "Failed to open JSON file: %s", filepath);
        return ESP_FAIL;
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp))
    {
        cJSON *root = cJSON_Parse(line);
        if (!root)
        {
            ESP_LOGW(TAG, "Skipping invalid JSON line");
            continue;
        }

        cJSON *task = cJSON_GetObjectItem(root, "task");
        cJSON *habit = cJSON_GetObjectItem(root, "habit");
        cJSON *event = cJSON_GetObjectItem(root, "event");

        esp_err_t err = ESP_FAIL;

        if (task)
        {
            err = ParseTasksJSON(task);
        }
        else if (habit)
        {
            err = ParseHabitsJSON(habit);
        }
        else if (event)
        {
            err = ParseEventsJSON(event);
        }
        else
        {
            ESP_LOGW(TAG, "Unrecognized object type in JSON line");
        }

        if (err != ESP_OK)
        {
            ESP_LOGW(TAG, "Parse function returned error");
        }

        cJSON_Delete(root);
    }
    ESP_LOGI(TAG, "Finished processing JSON file.");

    // Attempt to delete the file now that parsing is complete
    if (unlink(filepath) == 0)
    {
        ESP_LOGI(TAG, "Deleted file after successful parsing: %s", filepath);
    }
    else
    {
        ESP_LOGW(TAG, "Failed to delete file: %s (errno: %d - %s)", filepath, errno, strerror(errno));
    }

    fclose(fp);
    return ESP_OK;
}
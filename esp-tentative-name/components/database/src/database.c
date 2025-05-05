#include "database.h"

#include <sqlite3.h>

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

void DeleteDatabaseIfExists(char *database_path)
{
    static const char *TAG = "database::DeleteDatabase";

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
    static const char *TAG = "database::TestSDCard";

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

/// @brief Mounts the SD Card if not already mounted
/// @return ESP_OK on success, ESP_FAIL otherwise
static esp_err_t MountSDCard()
{
    static const char *TAG = "database::MountSDCard";

    if (sdcard_mounted)
        return ESP_OK;

    esp_err_t ret;
    sdmmc_card_t *card;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};

    ESP_LOGI(TAG, "Initializing SD card");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return ret;
    }

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
    ESP_LOGI(TAG, "Filesystem mounted successfully");
    // Log the card information
    sdmmc_card_print_info(stdout, card);

    return ESP_OK;
}

/// @brief Initializes the database and ensures the tasks table exists
/// @param db Database to be initialized. DATABASE MUST BE CLOSED MANUALLY.
/// @return SQLITE error code
int InitSQL(sqlite3 **db)
{
    static const char *TAG = "database::InitSQL";

    if (MountSDCard() != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to mount SD card.");
        return SQLITE_CANTOPEN;
    }
// Test SD card
#ifdef CONFIG_TEST_SD_CARD
    if (TestSDCard() != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to mount SD card.");
        return SQLITE_CANTOPEN;
    }
#endif

    sqlite3_initialize();

    // Build full path to database file
    char db_path[128];
    snprintf(db_path, sizeof(db_path), MOUNT_POINT "/calendar.db");
// DELETE FILE FOR DEBUGGING
#ifdef CONFIG_CLEAN_DATABASE
    DeleteDatabaseIfExists(db_path);
#endif

    ESP_LOGI(TAG, "Free heap: %lu bytes", esp_get_free_heap_size());
    // Open for read/write, create if not exists
    int rc = sqlite3_open_v2(db_path, db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

    if (rc)
    {
        ESP_LOGE(TAG, "database::InitSQL: Can't open database: %s", sqlite3_errmsg(*db) ? sqlite3_errmsg(*db) : "No error code available");
        return rc;
    }

    // -------------------------------------- Task Table ------------------------------------------
    ESP_LOGI(TAG, "database::InitSQL: Creating Task Table...");
    char *sql = "CREATE TABLE IF NOT EXISTS tasks ("
                "id TEXT PRIMARY KEY,"
                "name TEXT NOT NULL,"
                "datetime INTEGER NOT NULL,"
                "priority INTEGER NOT NULL,"
                "completed INTEGER NOT NULL,"
                "description TEXT);";
    char *zErrMsg;

    rc = sqlite3_exec(*db, sql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "database::InitSQL: SQL error: %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return rc;
    }

    // -------------------------------------- Event Table -----------------------------------------
    ESP_LOGI(TAG, "database::InitSQL: Creating Event Table...");
    sql = "CREATE TABLE IF NOT EXISTS events ("
          "id TEXT PRIMARY KEY,"
          "name TEXT NOT NULL,"
          "starttime INTEGER NOT NULL,"
          "duration INTEGER NOT NULL,"
          "description TEXT);";

    rc = sqlite3_exec(*db, sql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "database::InitSQL: SQL error: %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return rc;
    }

    // ------------------------------------- Habit Tables -----------------------------------------
    const char *sql_habits =
        "CREATE TABLE IF NOT EXISTS habits ("
        "id TEXT PRIMARY KEY, "
        "name TEXT UNIQUE NOT NULL);";

    const char *sql_entries =
        "CREATE TABLE IF NOT EXISTS habit_entries ("
        "habit_id TEXT NOT NULL, "
        "date TEXT NOT NULL, "
        "PRIMARY KEY (habit_id, date), "
        "FOREIGN KEY (habit_id) REFERENCES habits(id) ON DELETE CASCADE);";

    char *err_msg = NULL;
    if (sqlite3_exec(*db, sql_habits, 0, 0, &err_msg) != SQLITE_OK ||
        sqlite3_exec(*db, sql_entries, 0, 0, &err_msg) != SQLITE_OK) {
        ESP_LOGE(TAG, "SQL error: %s", err_msg);
        sqlite3_free(err_msg);
        return ESP_FAIL;
    }

    return SQLITE_OK;
}

int CloseSQL(sqlite3 **db)
{
    static const char *TAG = "database::CloseSQL";

    ESP_LOGI(TAG, "Closing database");
    int rc = sqlite3_close(*db);

    if (rc != SQLITE_OK)
    {
        ESP_LOGE(TAG, "database::CloseSQL: Can't close database: %s", sqlite3_errmsg(*db) ? sqlite3_errmsg(*db) : "No error code available");
        return rc;
    }

    return SQLITE_OK;
}

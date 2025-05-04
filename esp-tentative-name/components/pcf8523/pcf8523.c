#include <driver/i2c_master.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <time.h>
#include "esp_sntp.h"

#include "pcf8523.h"

#define I2C_MASTER_SCL_IO 6      /*!< GPIO number for I2C master clock */
#define I2C_MASTER_SDA_IO 10     /*!< GPIO number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0 /*!< I2C port number */
#define I2C_MASTER_FREQ_HZ 1000000
#define I2C_MASTER_TIMEOUT_MS 10000

#define I2C_TIMEOUT_MS 10000
#define MAX_RETRIES 3
#define RETRY_DELAY_MS 100

#define RTC_ADDRESS 0x68 // I2C address for PCF8523

#define SNTP_TIMEOUT_SEC 10 // Max time (seconds) to wait for NTP sync

#define CTRL1 0x08
#define CTRL2 0x00

// Global I2C handles
static i2c_master_bus_handle_t i2c_bus = NULL;
static i2c_master_dev_handle_t rtc_dev = NULL;

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)         \
    ((byte) & 0x80 ? '1' : '0'),     \
        ((byte) & 0x40 ? '1' : '0'), \
        ((byte) & 0x20 ? '1' : '0'), \
        ((byte) & 0x10 ? '1' : '0'), \
        ((byte) & 0x08 ? '1' : '0'), \
        ((byte) & 0x04 ? '1' : '0'), \
        ((byte) & 0x02 ? '1' : '0'), \
        ((byte) & 0x01 ? '1' : '0')

static uint8_t intToBCD(uint8_t num)
{
    return ((num / 10) << 4) | (num % 10);
}

static uint8_t bcdToInt(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// ----------------------------------------- Find I2C ---------------------------------------------
bool i2c_scan()
{
    static const char *TAG = "PCF8523::i2c_scan";

    // --- Configure I2C master bus
    i2c_master_bus_handle_t i2c_bus = NULL;
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_NUM,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    esp_err_t err = i2c_new_master_bus(&bus_config, &i2c_bus);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to create I2C master bus: %s\n", esp_err_to_name(err));
        return false;
    }

    esp_log_level_set("i2c.master", ESP_LOG_NONE);

    ESP_LOGI(TAG, "I2C scan start:\n");
    uint8_t dummy_data = 0x00;
    uint8_t valid_addr = 0;
    for (uint8_t addr = 1; addr < 127; addr++)
    {
        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = addr,
            .scl_speed_hz = I2C_MASTER_FREQ_HZ,
        };

        i2c_master_dev_handle_t temp_dev;
        err = i2c_master_bus_add_device(i2c_bus, &dev_cfg, &temp_dev);
        if (err == ESP_OK)
        {
            // Probe with empty write
            err = i2c_master_transmit(temp_dev, &dummy_data, 1, pdMS_TO_TICKS(100));
            if (err == ESP_OK)
            {
                printf("  Found device at 0x%02X\n", addr);
                valid_addr = addr;
            }
            i2c_master_bus_rm_device(temp_dev);
        }
    }

    esp_log_level_set("i2c.master", ESP_LOG_INFO);
    i2c_del_master_bus(i2c_bus);
    ESP_LOGI(TAG, "I2C scan complete.\n");
    if (valid_addr == 0)
    {
        ESP_LOGW(TAG, "Cannot find valid address!");
        return false;
    }
    else
    {
        ESP_LOGI(TAG, "Found I2C device at addr 0x%02X", valid_addr);
        return true;
    }
}

// -------------------------------------- Get Device I2C Bus --------------------------------------
esp_err_t InitRTC()
{
    static const char *TAG = "PCF8523::InitRTC";

    ESP_LOGI(TAG, "Creating bus...");
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false, // RTC has internal pull-up resistor
    };

    esp_err_t rc = i2c_new_master_bus(&bus_config, &i2c_bus);
    if (rc != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to install I2C bus: %s", esp_err_to_name(rc));
        return rc;
    }

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = RTC_ADDRESS,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    rc = i2c_master_bus_add_device(i2c_bus, &dev_config, &rtc_dev);
    if (rc != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add I2C device: %s", esp_err_to_name(rc));
        return rc;
    }
    ESP_LOGI(TAG, "rtc_dev initialized: %p", rtc_dev);

    vTaskDelay(pdMS_TO_TICKS(100));
    return ESP_OK;
}

// -------------------------------------- Read / Write --------------------------------------------
// Helper function to write a single register
esp_err_t rtc_write_register(uint8_t reg, uint8_t value)
{
    uint8_t data[2] = {reg, value};
    return i2c_master_transmit(rtc_dev, data, sizeof(data), pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
}

esp_err_t rtc_read_register(uint8_t reg, uint8_t *value)
{
    if (!value || !rtc_dev)
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err;

    err = i2c_master_transmit(rtc_dev, &reg, 1, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    if (err != ESP_OK)
    {
        return err;
    }

    err = i2c_master_receive(rtc_dev, value, 1, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    return err;
}

// ---------------------------------------- Configure Device --------------------------------------

// Reboot and configure control registers
esp_err_t RebootRTC()
{
    static const char *TAG = "PCF8523::RebootRTC";
    esp_err_t ret;

    ESP_LOGI(TAG, "Writing control registers...");

    ret = rtc_write_register(0x00, CTRL1);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to write CTRL1: %s", esp_err_to_name(ret));
        return ret;
    }

    uint8_t ctrl1;
    if (rtc_read_register(0x00, &ctrl1) == ESP_OK)
    {
        ESP_LOGI(TAG, "CTRL1: 0x%02X", ctrl1);
    }

    ret = rtc_write_register(0x01, CTRL2);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to write CTRL2: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "RTC control registers set successfully.");
    vTaskDelay(pdMS_TO_TICKS(100));
    return ESP_OK;
}

// ---------------------------------------- Read From Device --------------------------------------
esp_err_t pcf8523_read_time(time_t *out_time)
{
    static const char *TAG = "PCF8523::pcf8523_read_time";

    if (!out_time || !rtc_dev)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t start_reg = 0x00; // Begin reading from Control_1
    uint8_t data[10];

    for (int attempt = 0; attempt < MAX_RETRIES; ++attempt)
    {
        // Set register pointer to SECONDS
        esp_err_t err = i2c_master_transmit(rtc_dev, &start_reg, 1, -1);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "I2C transmit failed: %s", esp_err_to_name(err));
            return err;
        }

        // Read 7 bytes starting at SECONDS
        err = i2c_master_receive(rtc_dev, data, sizeof(data), -1);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "I2C receive failed: %s", esp_err_to_name(err));
            return err;
        }

        for (int i = 0; i < sizeof(data); i++)
        {
            ESP_LOGD(TAG, "%02xh: " BYTE_TO_BINARY_PATTERN "b", i, BYTE_TO_BINARY(data[i]));
        }

        // Check control register consistency
        if (data[0] != CTRL1 ||
            data[1] != CTRL2)
        {
            ESP_LOGW(TAG, "Control register mismatch: C1=0x%02X, C2=0x%02X, retrying read (%d/%d)...",
                     data[0], data[1], attempt + 1, MAX_RETRIES);
            vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
            continue;
        }

        // Check VL bit (bit 7 of seconds register)
        if ((data[0] & 0x80) == 0)
        {
            // VL bit not set, time is valid
            struct tm timeinfo = {0};
            timeinfo.tm_sec = bcdToInt(data[3] & 0x7F);
            timeinfo.tm_min = bcdToInt(data[4]);
            timeinfo.tm_hour = bcdToInt(data[5]);
            timeinfo.tm_mday = bcdToInt(data[6]);
            timeinfo.tm_mon = bcdToInt(data[8]) - 1;
            timeinfo.tm_year = bcdToInt(data[9]) + 100;

            *out_time = mktime(&timeinfo);
            if (*out_time == (time_t)-1)
            {
                ESP_LOGE(TAG, "mktime failed");
                return ESP_FAIL;
            }

            return ESP_OK;
        }
        else
        {
            ESP_LOGW(TAG, "VL bit set, retrying read (%d/%d)...", attempt + 1, MAX_RETRIES);
            vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
        }
    }

    ESP_LOGE(TAG, "Failed to read valid time from PCF8523 after %d attempts", MAX_RETRIES);
    return ESP_ERR_INVALID_STATE;
}

// ----------------------------- Get Current Time -------------------------------

esp_err_t SetTime()
{
    static const char *TAG = "PCF8523::GetTime";

    if (!rtc_dev)
    {
        ESP_LOGE(TAG, "RTC device not initialized. Call InitRTC() first.");
        return ESP_ERR_INVALID_STATE;
    }

    // --- Get UTC Time ---
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    time_t now;
    struct tm timeinfo;
    int retry = 0;

    if (pcf8523_read_time(&now) == ESP_OK)
    {
        struct tm *tm_info = localtime(&now);
        char buffer[64];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %I:%M:%S %p", tm_info);
        ESP_LOGI(TAG, "Current RTC Time: %s\n", buffer);
    }

    do
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        time(&now);
        localtime_r(&now, &timeinfo);
        retry++;
    } while (timeinfo.tm_year < (2022 - 1900) && retry < SNTP_TIMEOUT_SEC);

    if (timeinfo.tm_year < (2022 - 1900))
    {
        ESP_LOGW(TAG, "Time sync failed or timed out after %d seconds", SNTP_TIMEOUT_SEC);
        return ESP_ERR_TIMEOUT;
    }

    // --- Print Time ---
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    printf("Current time: %s\n", buffer);

    // --- Set RTC Registers ---
    uint8_t buf[8];
    buf[0] = 0x03;

    buf[1] = intToBCD(timeinfo.tm_sec);
    buf[2] = intToBCD(timeinfo.tm_min);
    buf[3] = intToBCD(timeinfo.tm_hour);
    buf[4] = intToBCD(timeinfo.tm_mday);
    buf[5] = timeinfo.tm_wday & 0x07;
    buf[6] = intToBCD(timeinfo.tm_mon + 1);
    buf[7] = intToBCD(timeinfo.tm_year % 100);

    return i2c_master_transmit(
        rtc_dev,
        buf,
        sizeof(buf),
        pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
}

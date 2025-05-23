#ifndef PCF8523_H
#define PCF8523_H

#include <time.h>
#include "esp_err.h"

#define UTC_OFFSET -7 * 3600

// Checks if there is a valid address to find for the I2C
// False on failure
bool i2c_scan();

// Initializes RTC I2C bus
esp_err_t InitRTC();

esp_err_t RebootRTC();

// Reads the RTC's time to a pointer
esp_err_t pcf8523_read_time(struct tm *time);
esp_err_t pcf8523_read_localtime(struct tm *localtime);

// Requires ESP internet connection
// Use SNTP to set the device's time
esp_err_t SetTime();

#endif

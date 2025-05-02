# pcf8523

Runs on hopes and dreams, and the PCF8523 RTC

# Configuration

RTC Pin     | Connection     |
------------|----------------|
 GND        | GND            |
 VCC        | 3.3V           |
 SDA        | GPIO10         |
 SCL        | GPIO6          |
 SQW        | NC             |

**Important:** Use 4.7k or 10k pull up resistors on SDA or SCL

SQW is a square wave generator

# Example implementation

```c
void app_main()
    // Setup network

    // --- PCF8523 DEMO ---
    if (!i2c_scan())
    {
        return;
    }
    ESP_ERROR_CHECK(InitRTC());
    ESP_ERROR_CHECK(RebootRTC());
    ESP_ERROR_CHECK(GetTime());
    while (1)
    {
        time_t rtc_time;
        if (pcf8523_read_time(&rtc_time) == ESP_OK)
        {
            struct tm *tm_info = localtime(&rtc_time);

            char buffer[64];
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %I:%M:%S %p", tm_info);
            printf("RTC Time: %s\n", buffer);
        }
        else
        {
            printf("Failed to read time from RTC.\n");
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // Enjoy having valid time
```
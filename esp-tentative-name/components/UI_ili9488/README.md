# LVGL using ILI9488 display
This creates the productivity device UI using LVGL and an ILI9488 SPI display using [atanisoft's driver and example](https://components.espressif.com/components/atanisoft/esp_lcd_ili9488/versions/1.0.11). Screen has been set to landscape mode and currently draws the UI. Interactability has yet to be implemented. This code was taken from [esp_port_lvgl's touchscreen example](https://github.com/espressif/esp-bsp/tree/76cc90336b34955fc76b510557b837e963b6a9e9/components/esp_lvgl_port/examples/touchscreen) and edited to work using the ILI9488 display.

## Usage
```
    /* LCD HW initialization */
    ESP_ERROR_CHECK(app_lcd_init());

    /* LVGL initialization */
    ESP_ERROR_CHECK(app_lvgl_init());

    /* All the GUI drawing */
    app_main_display();
    
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(5000)); // delay, can be adjusted
        lv_timer_handler();              // update screen
    }
```
```
timeDisplay(char * entry); //update time with string
```
```
create_task(const char * name, const char * dueDate); //add a task to the task list
```


## Font enabling
sdkconfig.default should have the necessary fonts enabled, but if it says there's fonts missing, you have to do `idf.py menuconfig`, go to Component Config > LVGL Configuration > Font Usage > Enable built in fonts and enable sizes 18, 24, and 48.

## Default pin assignments

For the ESP32C2 and ILI9488 screen pins are as follows:
| pin | usage |
| --- | ----- |
| VCC | 5V |
| GND | GND |
| TFT CS | 3 |
| TFT Reset | 4 |
| TFT DC | 5 |
| SPI MOSI (shared w/ sd card) | 20 |
| SPI CLK  (shared w/ sd card) | 7 |
| TFT LED Backlight | 2 |
| SPI MISO | unused |
| TOUCH_CLK | unused |
| TOUCH_CS | unused |
| TOUCH_DIN | unused |
| TOUCH_DO | unused |
| TOUCH_IRQ | unused |

Buttons are all on pin 1.
Voltage divider ladder is made based off of this [diagram](https://git.ucsc.edu/itphung/cse123-project/-/wikis/attachments/buttonDiagram.png).

If the library doesn't compile right, try adding idf_component.yml with the following. Note that the path I have database may be different from yours. At the moment requires database just for 1 function which shares the SPI interface with the SD card.
```
dependencies:
  idf: '>=4.4'
  atanisoft/esp_lcd_ili9488: ^1.0.11
  esp_lvgl_port:
    version: '*'
  lvgl/lvgl: ^9.2
  espressif/button: ^4.1.3
  database:
    path: ../database
```


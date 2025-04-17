# LVGL Example using ILI9488 display

This creates the productivity device UI using LVGL and an ILI9488 SPI display using [atanisoft's driver and example](https://components.espressif.com/components/atanisoft/esp_lcd_ili9488/versions/1.0.11). Screen has been set to landscape mode and currently draws the UI. Interactability has yet to be implemented. This code was taken from [esp_port_lvgl's touchscreen example](https://github.com/espressif/esp-bsp/tree/76cc90336b34955fc76b510557b837e963b6a9e9/components/esp_lvgl_port/examples/touchscreen) and edited to work using the ILI9488 display.
## Default pin assignments

For the ESP32C2 pins are as follows:
| pin | usage |
| --- | ----- |
| SPI MISO | 0 |
| SPI MOSI | 10 |
| SPI CLK | 8 |
| TFT CS | 3 |
| TFT Reset | 4 |
| TFT DC | 5 |
| TFT Backlight | 2 |

not 100% sure if TFT reset works since in the original code it was plugged into a weakly low strapping pin on the S3, but C3 does not have that function.

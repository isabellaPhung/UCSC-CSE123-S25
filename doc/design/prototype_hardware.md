## Prototype Hardware

The prototype consists of the microcontroller dev-board, an 3.5 inch LCD touch screen breakout board, a real-time clock breakout board, and several tactile buttons for navigating the UI of the device.

- **ESP32-C3-DevKit-RUST-1**: A microcontroller board featuring the ESP32-C3 SoC.
- **MSP3520**: An inexpensive, SPI TFT LCD touch screen that comes with an SD card reader.
- **Adafruit 3295**: A breakout board for the PCF8523 real time clock.

The wiring diagram is described by Figure \ref{proto_schem}

![Schematic of the prototype \label{proto_schem}](../../hardware/prototype_schematic.pdf)
Tactile buttons were used over the TFT's touchscreen capabilities due to a lack of GPIO pins on the ESP32-C3. 

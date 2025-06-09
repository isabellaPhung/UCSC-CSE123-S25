## Prototype Hardware

### Schematic

The prototype consists of the microcontroller dev-board, an 3.5 inch LCD touch screen breakout board, a real-time clock breakout board, and several tactile buttons for navigating the UI of the device.

- **ESP32-C3-DevKit-RUST-1**: A microcontroller board featuring the ESP32-C3 SoC.
- **MSP3520**: An inexpensive, SPI TFT LCD touch screen that comes with an SD card reader.
- **Adafruit 3295**: A breakout board for the PCF8523 real time clock.

The wiring diagram is described by Figure \ref{proto_schem}

![Schematic of the prototype \label{proto_schem}](../../hardware/prototype_schematic.pdf)
### Suggestions for improvement

Presented below are suggestions for selecting hardware components that overcome limits and design issues found in the prototype.
Suggestions are based on products that exist at the time of writing.

- Microcontroller:
  - Baseline: ESP32-C3 - 32-bit RISC-V core, 400 KB SRAM, 22 GPIOs
  - ESP32-S3 series - Xtensa LX7, 512 KB SRAM, 45 GPIOs
  - STM32H7 series - ARM Cortex-M7 & -M4, 564 KB to 1.4 MB RAM, 46 to 168 GPIOs
- Development boards:
  - Raspberry Pi Zero 2W - ARM Cortex-A53, 512MB of SDRAM
  - Teensy 4.1 - ARM Cortex-M7, 1024K RAM

The development kit used in the prototype ran into several limits: the number of available GPIO pins, and the memory availability
(more info on the memory in Section \ref{eval}).
The prototype ran into memory allocation issues when running the basic program loop.
A microcontroller with more RAM is necessary to operate the device without making compromises to the program in order to save on memory usage.
The prototype did not have enough GPIO pins to make use of touch controls, thus uses push-buttons for navigation.
More GPIO pins are necessary for enabling touch controls, along with implementing more features such as sound and vibration output.
SoCs like the STM32H7 with integrated graphics IP may be beneficial for better screen rendering performance.

- A higher resolution screen with more colors and capacitive touch input, as long as the communication protocol is supported by the controller.
- The real-time clock did not cause any difficulties in the design. The same module may be used.


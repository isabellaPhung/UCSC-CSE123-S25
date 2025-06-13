### Hardware {#suggestion_hw}

Presented below are suggestions for selecting hardware components that overcome limits and design issues found in the prototype.
Suggestions are based on products that exist at the time of writing.

- Microcontroller:
  - Baseline: ESP32-C3 - 32-bit RISC-V core, 400 KB SRAM, 22 GPIOs
  - ESP32-S3 series - Xtensa LX7, 512 KB SRAM, 45 GPIOs
  - STM32H7 series - ARM Cortex-M7 & -M4, 564 KB to 1.4 MB RAM, 46 to 168 GPIOs
- Development boards:
  - Raspberry Pi Zero 2W - ARM Cortex-A53, 512MB of SDRAM
  - Teensy 4.1 - ARM Cortex-M7, 1024K RAM

The development kit used in the prototype ran into several limits: the number of available GPIO pins, and the memory availability. At compile time, roughly 188kB of static memory was in use, with 133kB remaining for dynamic memory. When in use, the device had very little remaining free RAM which would be occupied when performing synchronization.
The prototype ran into memory allocation issues when running the basic program loop and in order to get all of the components working simultaneously, wifi and graphics buffers were reduced heavily, resulting in a much slower UI and synchronization process.
A microcontroller with more RAM is necessary to operate the device without making compromises to the program in order to save on memory usage.
The prototype also did not make use of multithreading, running entiirely on one thread which meant much less performance and the user could not interact with the device during synchronization.
The manufactured design will utilize multithreading, making for a much smoother user experience.
SoCs like the STM32H7 with integrated graphics IP may be beneficial for better screen rendering performance.

When synchronizing, the device would recieve the entirety of the database, even if the content wasn't changed. An HTTP server that responds to "changes since X" is a more efficient approach.
The prototype did not have enough GPIO pins to make use of touch controls, thus it uses push-buttons for navigation.
More GPIO pins are necessary for enabling touch controls, along with implementing more features such as sound and vibration output.
When synchronizing, the prototype would connect, recieve all of its data, then disconnect from the server, which made the process much slower. The manufactured design will utilize a persistent internet connection.
The prototype used SoftAP, but for the manufactured design bluetooth Wifi provisioning using a phone app makes for a better user experience.

- A higher resolution screen with more colors and capacitive touch input, as long as the communication protocol is supported by the controller.
- The real-time clock did not cause any difficulties in the design. The same module may be used.

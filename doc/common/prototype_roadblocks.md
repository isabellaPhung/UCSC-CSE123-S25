## Prototype Difficulties
The prototype design ran into memory availability issues
   188 kB of static memory (58.4% of available memory)
     66 kB used by graphics
     60 kB used by networking
     133 kB left for dynamic memory
ESP32C3 chip is insufficient for the manufactured product.

GPU option: If we utilized dual cpus for the graphical display, display would be faster
LVGL already custom external gpu rendering
With more memory and parallel processing, current approach should be more performant
This taught us our hardware requirements for the manufactured product
 
 An HTTP server that responds to "changes since X" is a more efficient approach.
   Wi-Fi provisioning + connection + MQTT = 100 kB (static \& dynamic)
   No memory = no persistent session = no stored messages
   User makes change $\rightarrow$ need to send entire backup

persistent sessions store non-ack'd qos = 1 messages
after a reconnect, non-ack'd items are re-sent instead of connecting and having a worker thread, the prototype connects, spends 5s, then disconnects, ending the session
Final product asks for the entire thing on power-on, then react to any new messages during the persistent connection.

MQTT: no persistent connection --- no message queue
ESP32-C3: RISC-V SoCs do not support memory expansion
Discuss potential alternatives: ESP32-S3, Teensy 4.1, STM32H7 line, Pi Zero 2W
Components: add haptics, audio, improve screen and touch
Additional features: time blocks, custom schedule organization
Although we can use SoftAP, Bluetooth Wifi provisioning using a phone app is a better user experience

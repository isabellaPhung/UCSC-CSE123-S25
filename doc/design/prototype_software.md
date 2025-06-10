### Software {#proto_sw}
The software for the ESP32-C3 was written in the C programming language, with the Espressif IoT Development Framework (esp-idf) for building, flashing, and testing the prototype.
The prototype makes use of the coreMQTT and mbedTLS libraries to establish a secure connection to the MQTT broker.
The prototype used LVGL to create a lightweight and fast C based GUI for display on the screen.
Buttons were implemented using the esp-iot button component.

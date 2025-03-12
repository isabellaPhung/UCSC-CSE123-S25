For code that requires the use of the ESP32C3, we'll be using pytest, which can compare expected output values directly to values from the microcontroller.
For high-level code that functions without access to the peripherals and low-level definitions specific to the microcontroller, unit tests will be created to test changes to these functions.
The esp-idf environment includes the unit testing framework 'Unity' which allows for easy creation, execution, and repeatability of these unit tests.

Some tests can be performed on the prototype, but additional testing on the manufactured product will have to be performed as well.
The following are tests that will be performed on both the prototype and the manufactured product:
- The display will be tested by sending an sample image to the LCD and performing a visual inspection to ensure that it displays as expected.
- Web connectivity can be tested through a simple ping test, ensuring that a device can send a signal through the server and to the device.
- Data persistence would include turning the device off, disconnecting power for 10 seconds before reconnecting it and verifying that data is retained when the device is reactivated via data comparison tests on pytest.
- The implemented scheduling algorithm will be given a variety of inputted schedules and events to determine if it is behaving as expected. It should be robust enough to handle edge cases such as impossible schedules or very complex schedules without crashing
- Data between both the device and the cloud database should be updated frequently while internet connection is maintained. This data will be compared to one another to test that data is not corrupted when transferring from one to another
- 
- In the event of full system failure or SD card data corruption, data should be updated and fully restorable on a fresh device.
- continued standard functionalitywhen not connected to the cloud. although this device can sync to the cloud so someone can check their phone for more information, this device can also be brought to discourage someone from relying on their phone as much.
- modularity testing, in the event a part is broken and the user could possibly send their unit in for repair, we want to ensure data and user settings are fully restored from the sd card
checking encryption to ensure data is fully encrypted end to end and is still decryptable via a replacement device

Qualitative analysis can then be made on user input to determine if the device can positively affect productivity.
The user would be provided the device and have it placed on their workspace for approximately two to three hours, afterwards they would be given a questionaire that would ask users to rate on a scale of -5 to 5 to determine how much the device improved their workflow.

The following are tests should be performed on a few samples of the manufactured product to ensure its construction quality:
- Checking the integrity of the device via tensile testing machines
- Half foot and one foot drop tests to verify the durability of the build
- surface chemical testing with standard househould products to ensure the combination of PVA and ABS plastics do not dissolve under exposure to these chemicals.

For high-level code that functions without access to the peripherals and low-level definitions specific to the microcontroller, unit tests will be created to test changes to these functions.
The esp-idf environment includes the unit testing framework 'unity' which allows for easy creation and execution of these unit tests.

When testing the software driving the device, three essential factors should be ensured: display, web connectivity, and data persistence.
- The display can be tested through sending an sample image to the LCD, and making sure that it displays as expected.
- Web connectivity can be tested through a simple ping test, ensuring that a device can send a signal through the server and to the device.
- Data persistence would include turning the device off, and disconnecting power and ensuring that data is retained when the device is reactivated.
These tests depend on variables that may change depending on what microcontroller was used, or what server provider was used.

Qualitative analysis can then be made on user input to determine if the device can positively affect productivity.
The user would be provided the device and have it placed on their workspace for a period of time, afterwards they would be given a feedback forum where they determine how much the device improved their workflow.

The following describe a series of testing programs and their expected behaviors necessary for certain tests. The host computer should be able to connect to the productivity device using a JTAG debug interface on the PCB.

test_sound.py - plays a sound from the speakers of the device and activates the vibration motors to verify these components work as expected.

test_screen.py - Screen displays a white background with different red targets that can be tapped to verify that the screen and touch functionality are working as expected.

test_batterylife.py - disables automatic power saving mode on low battery and runs a series of functions to emulate daily use of the device unsupervised. It will also begin a timer from the start of the test to when the device is unresponsive.

test_battery_cycling.py - cycles the device through 1000 battery charge and discharge cycles while monitoring the battery life. It also runs a series of functions to emulate daily use of the device unsupervised. Test is complete after 1000 battery charge cycles.

test_SoftwareUpdate.py - connects to the server to deliver a test version of the UI to be updated to the device to demonstrate the software update functionality.

test_clean.py - reads the device SD card and verifies that it is empty

function_test.py - performs a series of functions to emulate daily use of the device unsupervised. It should connect to the server and add various tasks and events, verify that they're updated on the device, perform focus mode on different tasks, and update habits.

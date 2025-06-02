### Persistence of data after power-off
Introduction: One of the key features of the productivity device is its offline usage, meaning that data must persist after being turned off and without internet access.

Scope: flash storage, file operations

Apparatus: productivity device, host machine for flashing and running tests

Independent variables: The flash device and model

Dependent variables: File contents (database), file size (bytes), power off behaviour (planned, power-loss)

Procedure:

1. Power on the device and verify it is connected to the internet by checking the wifi symbol in the top right. If the wifi symbol is not displayed, connect to the device's access point and provide it wifi credentials.
2. Create a test user on the web app. 
3. Log into the user via a laptop or mobile device. 
4. Add the device to the user using the corresponding device ID.
5. Add a task, event, and habit to be written to the device. Change the time setting from 12 hour to 24 hour display. 
6. Check the device to verify that the new schedule data has been added and the settings have been changed.
7. Disconnect the device from the internet in the settings menu. Return to the main menu and ensure the wifi symbol is not displayed.
8. Navigate around the device and check that the earlier data is still visible on the device.
9. Run the data_comparator.py pytest tool. This test program requests user data from the ESP device and compares the data to expected output.
10. Run the test to check the contents of the user data and compare it against the expected results.
11. Remove power to the device, wait 10 seconds before restoring power.
12. Rerun the test program and check that the results match the values prior to removing power.

Expectation: All file contents stay the same before and after powering off.

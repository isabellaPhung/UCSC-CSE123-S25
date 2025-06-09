### Factory Reset verification
Introduction: When issues arise in a device or if a user wants a fresh start, the productivity device must be able to be reset to a clean slate. 

Scope: flash storage, device operations

Apparatus: productivity device, host machine for flashing and running tests, laptop or mobile device for web app interaction

Independent variables: Device data and file contents 

Dependent variables: Existing user data on web app

Procedure:

1. Power on the device and check it is connected to the internet by checking the wifi symbol in the top right. If the wifi symbol is not displayed, connect to the device's access point and provide it wifi credentials. If the device is already connected to a user and has schedule information, skip to step 7.
2. Create a test user on the web app if it does not already exist. 
3. Log into the user via a laptop or mobile device. 
4. Add the device to the user using the corresponding device ID.
5. Add a task, event, and habit to be written to the device. Change the time setting from 12 hour to 24 hour display. 
6. Navigate the device to verify that the user's schedule data has been added and the settings have been changed.
7. Navigate to the device settings and select factory reset. The device will take a few moments to erase existing user data.
8. After the device reboots, navigate around the device and verify that the wifi symbol is not displayed and no existing events, tasks, or habits remain on the device.
9. On the web app, return to the devices tab and check that the device ID of the newly wiped productivity device is no longer listed under devices registered under the user.
9. Connect the device to the host machine and run test_clean.py. This program will verify that the device is free of all existing user data and settings.

Expectation: No existing schedule data, wifi information, or user settings. Web app updates to show the device is no longer registered under the user.

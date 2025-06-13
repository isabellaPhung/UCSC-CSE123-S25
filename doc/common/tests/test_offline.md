### Device Sync after Offline Use \label{offline_procedure}
Introduction: One of the key features of the productivity device is its offline usage and that the data from the device will be synchronized to the cloud after reconnection to the internet.

Scope: flash storage, file operations

Apparatus: productivity device, host machine for flashing and running tests

Independent variables: The flash device and model

Dependent variables: File contents (database), file size (bytes)

Procedure:

1. Power on the device and verify it is connected to the internet by checking the wifi symbol in the top right. If the wifi symbol is not displayed, connect to the device's access point and provide it wifi credentials. If the device already has existing a schedule and is registered under a user, skip to step 7.
2. Create a test user on the web app. 
3. Log into the user via a laptop or mobile device. 
4. Add the device to the user using the corresponding device ID.
5. Add a task, event, and habit to be written to the device. Change the time setting from 12 hour to 24 hour display. 
6. Check the device to verify that the new schedule data has been added and the settings have been changed.
7. Disconnect the device from the internet in the settings menu. Return to the main menu and ensure the wifi symbol is not displayed.
8. Navigate around the device and check that the earlier data is still visible on the device.
9. Interact with the device by setting one of the tasks to complete, deleting an event, and updating a habit. Wait 10 minutes.
10. Reconnect the device to the internet by navigating back to the settings and turning the wifi back on.
11. Refresh the web app after 10 seconds. The interacted with tasks, events, and habits should now be updated on the web app.

Expectation: The interacted with tasks, events, and habits should be updated on the web app after reconnection to the device within 10 seconds.

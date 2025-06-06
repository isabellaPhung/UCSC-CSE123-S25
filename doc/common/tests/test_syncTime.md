### Sync time 
Introduction: The productivity device periodically synchronizes its data with the server. One of the key design goals of this device was for the sync time to be under 10 seconds.

Scope: flash storage, file operations, cloud operations

Apparatus: productivity device, laptop or mobile device for web app interaction, timer

Independent variables: cloud data storage, flash storage

Dependent variables: File contents (database), power on behavior (planned)

Procedure:

1. Power on the device and verify it is connected to the internet by checking the wifi symbol in the top right. If the wifi symbol is not displayed, connect to the device's access point and provide it wifi credentials.
2. Create a test user on the web app. 
3. Log into the user via a laptop or mobile device. 
4. Add the device to the user using the corresponding device ID.
5. Add a task, event, and habit to be written to the device. 
6. Begin a timer when the device starts to sync and stop it once the device is updated with the new data. 

Expectation: Within 10 seconds, the device should be updated with the new schedule and setting information.

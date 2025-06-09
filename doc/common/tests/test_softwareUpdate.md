### Software Update test
Introduction: In order to provide security improvements and new features, the device needs to be able to recieve software updates over the internet.

Scope: device software, device operations

Apparatus: productivity device, host machine for flashing and running tests, laptop or mobile device for web app interaction

Independent variables: Device recieved contents 

Dependent variables: Device software version 

Procedure:

1. Power on the device and check it is connected to the internet by checking the wifi symbol in the top right. If the wifi symbol is not displayed, connect to the device's access point and provide it wifi credentials. If the device is already connected to a user and has schedule information, skip to step 7.
2. Create a test user on the web app if it does not already exist. 
3. Log into the user via a laptop or mobile device. 
4. Add the device to the user using the corresponding device ID.
5. Using the host machine for flashing and running tests, run test_softwareUpdate.py with the corresponding device ID in the following syntax: `python test_softwareUpdate.py [device_id]`. This will inform the server to prepare a software update for this particular device.
5. Back on the webapp, enter the device dashboard and navigate to the device settings. Select "Check for Updates". The webapp will send a signal the device to check for software updates. The device should display a window that asks the user to confirm the update with a button on the touch screen to press. Select the button to confirm the update.
6. The device will undergo a system update which will take some time. The device may restart multiple times. Once complete, the UI will be a different color to indicate the software update was sucessful.

Expectation: The device will show a different colored UI to demonstrate the software update has been completed succesfully. 

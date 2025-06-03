### Battery Life Cycle Testing
Introduction: While the device is intended to be generally stationary, the productivity device also includes a battery for portable use. Lithium ion batteries experience capacity degredation as they are charged and discharge throughout their lifespan. Most batteries experience 

Scope: Battery, Power

Apparatus: productivity device, host machine for flashing and running tests, charging cable, 1A power source

Independent variables: Device battery

Dependent variables: power on behaviour, device interaction

Procedure:

1. Power on the device and ensure it is at full charge as shown by the battery indicator in the top right of the UI. If it is not fully charged, charge the device to full.
2. Disconnect the device from the power cable. Check the settings to ensure the device is not set to power saving mode. Connect the productivity device to the host machine.
2. Run the test_batterylife.py pytest tool. This test program will disable charging from the host to the productivity device, automatic power saving mode on low batterin, and will run a series of functions to emulate daily use of the device. It will also begin a timer from the start of the test to when the device is unresponsive.
3. Leave the device on and undisturbed. The screen should remain on for the entire duration of the test.
4. Check on the test setup every three hours to monitor its battery life. Test ends when the device runs out of charge and the test_batterylife.py tool returns a final time on the device battery life duration.

Expectation: The device should not run out of charge for at least four hours.

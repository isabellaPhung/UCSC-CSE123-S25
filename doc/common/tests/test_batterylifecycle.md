### Battery Life Cycle
Introduction: While the device is intended to be generally stationary, the productivity device also includes a battery for portable use. Lithium ion batteries experience capacity degredation as they are charged and discharged throughout their lifespan. Battery degradation from 100% down to 80% can be seen anywhere between 500 to 2000 charging cycles. To ensure that the device doesn't experience severe battery life fall off, battery cycling testing is performed to ensure charge management circuitry functionality and battery quality.

Scope: Battery, Power

Apparatus: productivity device, host machine for flashing and running tests, charging cable, 1A power source

Independent variables: Device battery

Dependent variables: power on behaviour, device interaction

Procedure:

1. Power on the device and ensure it is at full charge as shown by the battery indicator in the top right of the UI. If it is not fully charged, charge the device to full.
2. Connect the productivity device to the host machine.
3. Run the test_battery_cycling.py pytest tool. This test program will cycle the device through 1000 battery charge and discharge cycles while monitoring the battery life. Place the test setup in a room temperature area to be undisturbed for multiple weeks. Check on the test setup once a day to ensure the test has not halted and the test is proceeding as expected. Test is complete after 1000 battery charge cycles.

Expectation: The device battery life degradation should not exceed 20% after 1000 battery charge cycles.

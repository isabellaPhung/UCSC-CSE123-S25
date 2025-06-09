Some tests require programs to automate usage of the productivity device. This testing suite should use [pytest](https://docs.pytest.org/en/stable/), which can compare expected output values directly to values from the microcontroller.
The ESP-IDF environment includes the unit testing framework [Unity](https://github.com/ThrowTheSwitch/Unity) which allows for easy creation, execution, and repeatability of unit tests.

Some tests can be performed on the prototype, but additional testing on the manufactured product will have to be performed as well.
The following are tests that will be performed on both the prototype and the manufactured product:
- test_bootTime.md
- test_cloud.md
- test_manyToOne.md
- test_offline.md
- test_persistence.md
- test_restoration.md
- test_security.md
- test_soundhaptics.md
- test_syncTime.md
- test_wifi.md

The following are tests should be performed on a few samples of the manufactured product to ensure its construction quality and additional features beyond the prototype.
- test_batterylife.md
- test_batterylifecycle.md
- test_chargespeed.md
- test_chemical.md
- test_drop.md
- test_emi.md
- test_factoryReset.md
- test_lcd.md
- test_portDurability.md
- test_softwareUpdate.md
- test_tensile.md
- test_thermal.md
- test_thermalShock.md
- test_UV.md
- Qualitative analysis should also be performed to determine if the device can positively affect productivity where a user would be provided the device and have it placed on their workspace for approximately two to three hours, afterwards they would be given a questionnaire that would ask users to rate on a scale of -5 to 5 to determine how much the device improved their workflow.

Some tests require programs to automate usage of the productivity device. This testing suite should use [pytest](https://docs.pytest.org/en/stable/), which can compare expected output values directly to values from the microcontroller.
The ESP-IDF environment includes the unit testing framework [Unity](https://github.com/ThrowTheSwitch/Unity) which allows for easy creation, execution, and repeatability of unit tests.

Some tests can be performed on the prototype, but additional testing on the manufactured product will have to be performed as well.
The following are tests that will be performed on both the prototype and the manufactured product:

- Boot time
- Data cloud synchronization
- Many Users to One Device
- Device Sync after Offline Use
- Persistence of data after power-off
- User data restoration on new device/Many devices to one user
- Secure connection
- Modifying entries to and from the device.
- Sync time 
- Wifi Connection
- Cloud synchronization updates: basic
- Cloud synchronization updates: dual updates
- Cloud synchronization updates: undoing edits

The following are tests should be performed on samples of the manufactured product to ensure its construction quality and additional features beyond the prototype.

- Battery Life
- Battery Life Cycle
- Charge Speed
- Drop Testing
- Electromagnetic interference and electromagentic compatability
- Factory Reset verification
- LCD functionality Verification
- Haptics and Sound functionality Verification
- Port Durability
- Software Update verification
- Tensile Strength testing
- Thermal Cycling: Cold and Dry Heat
- Thermal Shock: Air to Air
- UV Exposure Testing
- Qualitative analysis should also be performed to determine if the device can positively affect productivity where a user would be provided the device and have it placed on their workspace for approximately two to three hours, afterwards they would be given a questionnaire that would ask users to rate on a scale of -5 to 5 to determine how much the device improved their workflow.

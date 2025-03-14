### Persistence of data after power-off
Scope: flash storage, file operations

Apparatus: productivity device, host machine for flashing and running tests

Independent variables: The flash device and model

Dependent variables: File contents (database), file size (bytes), power off behaviour (planned, power-loss)

Procedure:

1. Before booting up the device, verify it is not connected to the internet to ensure the device is not paired with the internet and that data is not overwritten on the device.
2. Create a test user and some schedule data to be written. This data is automatically written to the sd card.
3. Run the data_comparator.py pytest tool. This test program requests user data from the ESP device and compares the data to expected output.
4. Run the test to check the contents of the user data and compare it against the expected results.
5. Remove power to the device, wait 10 seconds before restoring power.
6. Rerun the test program.

Expectation: All file contents stay the same before and after powering off.

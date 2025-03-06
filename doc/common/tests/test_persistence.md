### Persistence of data after power-off
Scope: flash storage, file operations

Apparatus: The device, host machine for flashing and running tests

Independent variables: The flash device and model, flash storage interface software

Dependent variables: File contents (randomized bytes, database), file size (bytes), power off behaviour (planned, power-loss)

Procedure:

1. Create a stream of bytes to be written to the storage medium. Keep a record of the contents and the length of the stimulus.
2. Create a test program for the flash storage component that writes the given stimulus to the storage medium, then the device powers off safely.
3. Create a similar test, where the device powers off unsafely.
4. Run the test to write to the flash storage.
5. Check the contents of the flash storage. Ensure that 100% of the bytes written match what is read out.

Expectation: All file contents stay the same before and after power off.

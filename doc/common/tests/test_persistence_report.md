### Persistence of data after power-off test report
\noindent
**Test Purpose**:
This is intended to verify that the device is capable of retaining user data after shutdown without internet connection.

\noindent
**Test Procedure**:
Exact testing procedures can be found at \ref{persistence_procedure}. Results are visually observed.

\noindent
**Test Results**:

- Device had existing user data on it from \ref{cloud_test} so test is begun from step 7
- Device was disconnected from the internet by powering off the hotspot it was connected to
- After pressing the synch button, the wifi symbol in the top right disappeared
- Device was then unplugged for 10 seconds before plugging the power cable back in
- After the device powered on, device was confirmed to not be connected to the internet by visually checking for the wifi symbol
- The device displayed the same user data as before it was powered off

\noindent
**Final Analysis**:
The device passes. It is able to operate retain user data after losing wifi connection and being powered off and on.

### Cloud sync: simultaneous edits

**Test Purpose**:
To test concurrent edits to a task from the device and web app.

\noindent
**Test Method**:
Add a task on the web app tasks pane.
Press the sync button on the device to get the task from the cloud.
Turn off the Wi-Fi access point that the device is connected to temporarily.
Mark the task as deleted on the web app, and completed on the device.
Turn on the Wi-Fi access point, and reconnect to it from the device (use provisioning if needed).
Press the sync button on the device.

\noindent
**Test Results**:
TODO

\noindent
**Final Analysis**:
The device is able to handle when the completion status of a task is different locally and on the cloud.

### Cloud sync: basic edits

\noindent
**Test Purpose**:
To test edits to a task from the device.

\noindent
**Test Method**:
Add an incomplete task on the web app tasks pane.
Press the sync button on the device to get the task from the cloud.
Mark the task as completed on the device.
Press the sync button on the device.
Verify that the task is complete on the web app.

\noindent
**Test Results**:
The web app shows a completed task.

\noindent
**Final Analysis**:
The web server is able to handle requests for modifying a task coming from the device.

### Cloud sync: undoing edits

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
Mark the task as incomplete on the device.
Press the sync button on the device.
Verify that the task is incomplete on the web app.

\noindent
**Test Results**:
The web app shows that the corresponding task in not complete.

\noindent
**Final Analysis**:
The web server is able to handle requests for incompleting a completed task.

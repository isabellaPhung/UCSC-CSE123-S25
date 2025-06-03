The first part of the demonstration will be done assuming a device that has been pre-configured.

### Part 0: Description of the data flow {-}

Quickly go over the different services used, and how task data travels between each node of the data flow chart.
The goal is to familiarize the audience with what each node does.

### Part 1: Demonstration of the user interface {-}

Demonstrate what each tab of the user interface does.

1. Main screen: describe the purpose of the focus mode and each element that is on display
2. Explain that the main page only shows today's data.
3. Tasks list: Show a sample list of tasks (both the main page and calendar)
4. Habits tracking: explain how to read the habits page
5. Events: explain that the events pane shows events

### Part 2: Demonstration of task modification {-}

Demonstrate the different types of task modification possible on the web interface.

- Add a new task
  1. In a web browser, navigate to the web interface and log-in with the user credentials:
    - Username: bob
    - Password: password
    - Device ID: 55
    - Name: my device
  2. Once logged in, create a new task with the following fields:
    - Name: Mechatronics final project
    - Description: Attend team meeting
    - Priority: High 
    - Due date: Today
    There will be one of each already on the device: habit, task, event
  3. Save the task
  4. From the device, manually request a refresh of the local tasks list through the UI.
  5. Navigate to the tasks page in the device UI. The new task should pop up corresponding to the due date.

- Modify an existing task
  1. On the device, mark the previously created task as "complete"
  2. Allow the device to send the task update to the broker.
  3. Verify that on the website, the task is marked as completed.

- Double edit
  1. In the web interface, mark the task as deleted.
  2. On the device, mark the task as complete.
  3. Let the device send the new completion status.
  4. Show on the web interface that the task has been deleted.
  5. Show on the client that the task has been deleted

- Event delete
  1. On the device, delete an event
  2. Let the device sync
  3. Show on the web interface that the event has been deleted.

### Part 3: Setup procedure {-}

Describe the procedure of setting up a new device after purchase.

1. The user receives the device, with an instruction slip packaged together with the product.
2. The user navigates to the website, and creates an account.
  - Username: jdoe
  - Password: password
3. Once logged in, the user is presented with the device setup page. The user then enters their device's ID, found on the device itself, into the menu.
  - Device ID: 54
  - Name: my device
4. The device should be mapped to that user, and will be intractable once selected in the menu.
5. The user can now enter the main menu of the linked device. This pulls from the data in the cloud storage of that ID.
6. The user powers on the device, which looks for network configuration data in its storage. If not found, it enters access point mode and creates a https server.
7. The user can connect to the device, and enter their wifi credentials.
8. The device attempts a connection, if successful, will continue operations as normal.

### Part 4: Broken Product replacement {-}

Since each device is tied to a user id and has a backup in the database mapped to its id, copying the tasks from one device to another is fairly trivial.
The user must set up the device as usual.
Then, the user can select the copy data menu option in the web interface.
After the server verifies that the user owns both devices, it gets the backup from the old device and overwrites the new device's backup.
The new device can operate as normal without knowledge of the old device.

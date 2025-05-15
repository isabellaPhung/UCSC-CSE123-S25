The first part of the demonstration will be done assuming a device that has been pre-configured.

### Part 0: Description of the data flow {-}

Quickly go over the different services used, and how task data travels between each node of the data flow chart.
The goal is to familiarize the audience with what each node does.

### Part 1: Demonstration of the user interface {-}

Demonstrate what each tab of the user interface does.

1. Main screen: describe the purpose of the focus mode and each element that is on display
2. Tasks list: Show a sample list of tasks (the contents of `example_backup.json`)
3. Habits tracking: explain how to read the habits page

### Part 2: Demonstration of task modification {-}

Demonstrate the different types of task modification possible on the web interface.

- Add a new task
  1. In a web browser, navigate to the web interface and log-in with the user credentials:
    - Username: bob
    - Password: password
  2. Once logged in, create a new task with the following fields:
    - Name: Mechatronics final project
    - Description: Attend team meeting
    - Priority: Medium
    - Due date: May 29th, 2025 23:59
  3. Save the task
  4. From the device, manually request a refresh of the local tasks list through the UI.
  5. Navigate to the tasks page in the device UI. The new task should pop up corresponding to the due date.

- Modify an existing task
  1. In the web interface, select the task added in the previous step.
  2. Edit the name field of the task to "Mechatronics presentation"
  3. Manually refresh on the device, and verify to the audience that the name has changed, while the other fields remain the same.

- Remove a task
  1. In the web interface, select the task modified in the previous step.
  2. Set the status of the task to "deleted"
  3. Manually refresh on the device, and verify to the audience that the task no longer exists.

- Double edit
  1. In the web interface, create the same task from the first step.
  2. Manually refresh on the device.
  3. On the device, mark the task as complete.
  4. In the web interface, mark the task as deleted.
  5. Show on the web interface (or S3 bucket) that the task has been deleted.

### Part 3: Setup procedure {-}

Describe the procedure of setting up a new device after purchase.

1. Once the user has received the device, they will find an instruction booklet enclosed together with the product.
2. Once powered on for the first time, the device will look for configuration data in it's storage. If not found, the device enters setup mode and acts as https server.
3. Login to the web interface, and click on the add device menu option. This will give the user an api key to give to the device.
4. With a companion app or a web browser, access the device's web page and enter the api key.
5. The device will attempt to connect to the cloud endpoint using the api key. Once the device has successfully received an updated tasks list, the device will enter normal operation.

### Part 4: Broken Product replacement {-}

Since each device is tied to a user id and has a backup in the database mapped to its id, copying the tasks from one device to another is fairly trivial.
The user must set up the device as usual.
Then, the user can select the copy data menu option in the web interface.
After the server verifies that the user owns both devices, it gets the backup from the old device and overwrites the new device's backup.
The new device can operate as normal without knowledge of the old device.

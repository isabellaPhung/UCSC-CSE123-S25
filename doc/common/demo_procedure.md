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
  1. On the device, mark the previously created task as "complete"
  2. Allow the device to send the task update to the broker.
  3. Verify that on the website, the task is marked as completed.

- Remove a task
  1. In the web interface, select the task modified in the previous step.
  2. Set the status of the task to "deleted"
  3. Manually refresh on the device, and verify to the audience that the task no longer exists.

- Double edit
  1. In the web interface, create the same task from the first step.
  2. Manually refresh on the device.
  3. In the web interface, mark the task as deleted.
  4. On the device, mark the task as complete.
  5. Let the device send the new completion status.
  6. Show on the web interface that the task has been deleted.

### Part 3: Setup procedure {-}

Describe the procedure of setting up a new device after purchase.

1. The user receives the device, with an instruction slip packaged together with the product.
2. The user navigates to the website, and creates an account.
  - Username: 
  - Password: 
3. Once logged in, the user is presented with the device setup page. The user then enters their device's ID, found on the device itself, into the menu.
  - Device ID: 
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

### Part 5: Prototype versus Manufactured product {-}

Caddy would be replaced with NGINX.
S3 would be replaced with PostgreSQL.
Flask would be replaced with Custom Framework.
ESP32 would be replaced with Custom microcontroller PCB.
MQTT would be replaced with HTTP.

The justification for replacing the Caddy with NGINX in the large scale is that NGINX supports large scale optimization, handles concurrent connections, and supports the high-traffic intensity required for commercial products.
S3 object storage must be replaced with PostgreSQL to accomadate for complex quiries, joins and relational data. Its essential for real timeoperations.
Flask must be replaced in order to account specialized throughput and secruity requirments. 
The Esp32 must be replaced for the Manufactured product with a costum microcontroller or PCB so that the product is designed with optimzed size, power consumtion and physical design.
MQTT should be replaced with the HTTP protocol for universality and for the leveraging of exisiting web infastructure. 

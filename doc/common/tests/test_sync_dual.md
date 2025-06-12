### Cloud synchronization updates: dual updates 
Scope: cloud operations, data store and overwrite

Apparatus: Device, cloud server dashboard

Independent variables: connection protocol, cloud data storage.

Dependent variables: Update packets[id, completion status], cloud storage task list

Procedure:

1. Clear a device's data on the cloud storage then add a single task on the web dashboard.
2. Allow the device to receive the task list from the cloud.
3. Disconnect the device from the internet, and mark the task as completed.
4. On the cloud storage, manually set the task to be deleted.
5. Re-connect the device to the internet, and allow it to sync to the server by sending an update.

Expectation: Since the deleted status has higher precedence than the completed status, the server recieves the completed status and ignores it. 
Then, it sends its updates to the device, making the task set to deleted on the device as well.

### Cloud synchronization updates: undoing edits
Scope: cloud operations, data store and overwrite

Apparatus: Device, cloud server dashboard

Independent variables: connection protocol, cloud data storage.

Dependent variables: Update packets[id, completion status], cloud storage task list

Procedure:

1. Manually upload a list of tasks containing only one task which is incomplete to the cloud data storage.
2. Complete the task on the device and update the cloud storage with its status.
3. On the device, update the status to be incomplete again.
4. Allow the cloud and device to sync. 

Expectation: The cloud's tasks list should be set back to be incomplete when the device updates it.

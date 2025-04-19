### Cloud synchronization updates: basic
Scope: cloud operations, data store and overwrite

Apparatus: Device, cloud server dashboard

Independent variables: connection protocol, cloud data storage.

Dependent variables: Update packets[id, completion status]

Procedure:

1. Manually upload a list of tasks containing only one task which is incomplete to the cloud data storage.
2. Send an update to the cloud to complete the task with the same ID.
3. Verify that the cloud entry point received the massage.
4. Verify that the message was routed to the correct services, and that the task list in the cloud storage has been modified.
5. Verify that only the task with the same ID has been modified.

Expectation: Within 10 seconds, the cloud storage should contain the updates tasks list with the one task having the completed status.

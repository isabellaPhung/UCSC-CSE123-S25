The lambda function on aws should trigger when the IoT endpoint receives a publish from any device ID that is not "server".
When the "action" field is "update", then the json in storage should have its completion fields updated.
When the "action" field is "refresh", then the json in storage should be published in chunks for the device to update its local list of tasks.

- Since the device will have limited memory, the list of tasks should be received in chunks, as the tasks database may be massive.
  - Each task could have an "index" field, which the last will have the value "last" instead of the number
- Another potential solution could be to send a limited buffer of tasks as discussed earlier (7 days of due dates)
  - This is limited as tasks due on the 8th day from now will not be updated until the next refresh, which could be on the 7th day.

The SQL for the message routing for refresh is
`SELECT id, type FROM 'iotdevice/+/datas3' WHERE action='refresh' AND (type='task' OR type='habit' OR type='event')`

The message to the server must be:
```json
{
  "id":"55",
  "action":"refresh",
  "type":"task"
}
```
Where "type" is allowed to be "task", "event", or "habit"

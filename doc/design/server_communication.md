## Device-server Communication

### Entry Serialization

The server and device must agree on a set format for entries in order to communicate effectively.
The three entry types --- tasks, events, and habits --- are designed, and serialized in a way that is most compatible with the different data structures that are in use.
The JSON format should be used as it has widespread support.
Thus, text-based protocols should be used.

All entry types have a unique identification and a name (limited to 32 characters).
Tasks contain an UUID, name, description, completion status, priority, and due date. 
The description can be used in addition to the name to describe the task; its length is limited to 1024 characters.
The completion status can be set as incomplete, complete, or deleted.
When a task is set to be deleted, any changes made to it are to be ignored, and should not be displayed on any client.
The priority ranges from 1 to 3, to be used as described in Section \ref{entry-design}.
The due date is encoded as a unix timestamp by when the task is to be completed.

Events contain an UUID, name, a start time, and a duration.
The start time is a unix timestamp of when an event begins.
The duration is the amount in seconds from the start time when the event ends.

Habits contain an UUID, name, goal, and completion status.
The goal is encoded as a bit-mask of the 7 days of the week --- the least significant bit corresponding to Saturday.
For example, a habit goal of Tuesdays and Thursdays would be encoded as `0b0010100` or 20.
The completion status is a list of unix timestamps for the day a habit was completed.
To be consistent, the 0th second of that day is used. 

### Protocol

The data transferred between the device and the server must be encrypted to respect the privacy of the user.
Several text-based protocols support this, such as HTTPS or MQTT.
The device will make requests to the server to get the entire data backup, or incremental backups since a reference timestamp.
Any modifications made on the device will be sent to the server with the corresponding UUID, and the modified fields.
Packets being sent to or from the server should come with the entry serialization, as well as the following:
source ID, action type, and data type.
The server will have the ID of "server".
The source ID is the UUID of the device, which is pre-configured at the manufacturing time.
The action type can be either refresh, update, response, or ack.
Requesting to the server for the full backup is done with the refresh action, which gets the response action back from the server.
Requesting for changes to be made to entries is done with the update action, which gets the ack action back from the server.

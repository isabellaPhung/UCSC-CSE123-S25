| Design Objective     | Units   | Target/Range |
|----------------------|---------|--------------|
| Price                | USD     | $150         |
| Devices per user     | Units   | 10           |
| Users per device     | Units   | 10           |
| Cloud sync time      | Seconds | 100m         |
| Simultaneous entries | Entries | 16000        |
| Database Size        | Bytes   | 32M          |
| Standby Battery life | Hours   | 168          |
| Wakeup time          | Seconds | 100m         |

[comment]: <> (Assuming entries are at most 2k in size, (32 * (1024 * 1024)) / (2048) = 16384. rounding down wo 16000 leaves 768k for other storage uses.)
[comment]: <> (Would these calculations need to be in the actual document content?)

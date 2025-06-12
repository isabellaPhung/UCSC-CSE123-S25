## Device UI Specifications
The device UI is developed with a similar style guide to the web app and phone app. Schedule units, whether they be tasks, habits, or events, must be formatted in similar manners across all devices for ease of use and continuity.

Tasks are displayed in a manner that is easy for the user to view and access them, preferably in the form of a list showing the due date with some way to differentiate between completed and incomplete tasks, either with color or a symbol. 

When selected or tapped on, the task displays either a different menu or expands in some way for the user to view its details: the description, its priority, its due date, as well as be able to interact with the task by deleting or focusing the task. An example is shown in fig. \ref{taskdetail}.

![Task Detailed View\label{taskdetail}](taskTile.png){ height=40% }

Entering focus mode halts all synchronization tasks so as to save on battery and displays the current date and time, a timer that counts down for the task, and options for the user to exit out of focus mode, restart the timer, or set the current focused task to complete. The timer for this focus mode can be changed in the user settings. An example of focus mode is shown in fig. \ref{focusmode}

![Focus Mode \label{focusmode}](focusTile.png){ height=40% }

Completed tasks are displayed until some duration in which they are deleted from the server and the device. This duration can be adjusted according to user preference in the settings. If a user decides to delete a task, a confirmation screen appears to ensure the user has not missclicked. If a task is deleted, it is immediately removed from the device UI. 

Events are also be displayed in an easy to view manner, preferably as a list. Events mirror tasks in that, when selected, they also display details regarding the event and options such as deleting the event. Events are not be displayed in the list if they have already passed. Similar with tasks, events show a deletion warning before confirmation and they are deleted immediately.

An example of the tasks and events menu is shown in fig. \ref{taskevent}. While the task and event views were combined into one screen, they can be separated in to multiple screens.

![Task and Event Menu \label{taskevent}](taskEvent.png){ height=40% }

An optional additional view with be a calendar that displays all of the events as well as tasks is also recommended.

Habits are displayed going right to left, with the rightmost being the current day and the left most being 6 days prior to the current day. This gives 6 days for the user to check off if they have performed a habit. The list of days is updated at midnight of the user's time. An example of the habits menu is shown in fig. \ref{habits}. Different colors are used to differentiate between the days where the habit _should_ be done and days when the habit _has been performed_.

![Habit Menu \label{habits}](habitTile.png){ height=40% }

Depending on the amount of available RAM in the device, schedule units can be displayed in groups at a time and updated when the user scrolls down. An alternative is loading a specific number of schedule units and using arrow buttons to load more tasks and events.

When the device is being set up for the first time, a screen instructing the user to download the phone app and connect to the device via their phone's bluetooth is displayed, similar to fig. \ref{wifisetup}. This screen is only ever shown during first time setup out of the box or after a factory reset. After setup when the device loses internet connection, it will continually search for bluetooth connection while still allowing the user to continue using the device unless the user turns the device's wifi off in the settings menu.

![Wifi Setup \label{wifisetup}](wifi_setup.png){ height=40% }

The UI must display some manner of wifi symbol on the screen when the user is viewing their schedule to indicate when the device is connected. In addition, a synchronization symbol is displayed when the device is synchronizing with the server. 

Pop ups appear on the lower area of the screen prompting the user if they want to enter focus mode for certain tasks if the user has enabled the guided time block system.

The UI must also adapt according to the user configuration settings. Color theme settings change the colors of the UI, text size settings scale the text, and changing the timezone and time display settings change the time values on the device appropriately. 

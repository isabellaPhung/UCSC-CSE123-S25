## Webpage Design Philosophy

The schedule companion requires a web application that will provide the user with a medium to manage user-to-device settings, manage user data, and view or edit scheduled items. This section of the document provides the principles that are necessary to the design of this webpage. These principles revolve around the categories defined in entry design and their actionable functionality within the user interface. It also includes necessary applications for the user interface such as the login, device management, and calendar pages. 


**Authentication and User Management:**

- User Authentication through login page
    - Sign up option within login page to register new user
        - Create account with full name, username, email, and password
            - Username uniqness check
            - Password strength validation with visual feedback
            - Automatic user ID generation 
    - Forgot password option to reset credentials
    - Prompts user for email and password 
        - Credential validation against existing user
        - Session token managment with CSRF protection
        - Secure login state across browser session
        - Sncludes persistent login state across browser sessions

**Device management**

- Device Registration
    - Add device with unique device ID
        - Device will utilize wifi to link to webserver
        - Option to link device to user account
    - Switch between user devices 
    - Remove device from user directory
- Device State Persistence
    - Remember selected device across sessions
    - Handle device availability and connection status
    - User data linked synced to device
    

**Main Page**

- Access to tasks, habit, event, calendar, device managment, and login page
- Add, delete or complete Tasks, habits or events
- Display of today's tasks, habits and events with their times
    - Times implemented with Unix timestamps
    - View task description, priority, completion and due times
    - View event duration and due date

**Tasks** 

- Addition
    - Prompts user for task name, due date, due time, priority, and description
        - Set priority level using numeric values 1-3
            - Initialize completion status as 0 (incomplete)
        - Define due date and specific time (hour, minute, AM/PM)
            - Convert time selection to Unix timestamp for storage

- Management
    - Toggle completion status: 0 (incomplete) - 1 (complete)
    - Soft delete functionality: set completion status to 2 (deleted)
    - Maintain completion state persistence across sessions
- Display
    - Filter tasks by date range (today, specific dates)
    - Sort by completion status first, then priority level
    - Visual priority indicators with color coding
    - Expandable detail view showing full task information
    - Real-time timestamp display and conversion to readable format

**Habits**

- Creation 
    - Prompts user with day of the week display that allows user to select days when habit should be fullfilled
    - Prompts user for habit name
    - Add and remove habit options 
- Definition
    - Citwise flags to define weekly schedule
    - Flag mapping: Sunday=0x40, Monday=0x20, Tuesday=0x10, Wednesday=0x08, Thursday=0x04, Friday=0x02, Saturday=0x01
    - Calculate combined flags for multiple day selection
- Completion Tracking
    - Track daily completion status per habit
    - Maintain completion history across different dates
    - Visual distinction between scheduled versus non-scheduled days
- Daily Habit Display
    - Provide full weekly grid view in dedicated habits page
    - Current day highlighting in weekly view
    - Completion status visualization with checkboxes

**Events**

- Creation 
    - Prompts user for event name, event date, start time, priority, duration, and description
    - Define start date and specific time (hour, minute, AM/PM)
    - Convert time selection to Unix timestamp for storage
- Status Tracking
    - Determine "active" events (currently happening based on start time + duration)
    - Track event completion or cancellation
    - Soft delete functionality for events
- Display
    - Show events by date range
    - Display calculated duration and end times
    - Visual indicators for currently active events
    - Time-based sorting of events

**Calendar**

- Monthly Calendar Display
    - Generate calendar grid for any month/year
    - Properly handle month boundaries and leap years
    - Navigate between months with controls
- Highlight current day across all calendar views
    - Task-calendar integration
    - Display tasks on their scheduled dates within calendar
    - Limit visible tasks per day with overflow indicators
    - Handle multiple tasks on same date
    - Click-through functionality to detailed day view
- Day View Modal
    - Show all tasks and events for selected date
    - Sort items chronologically by time
    - Full detail expansion for each item
    - Filter by exact date match using timestamp ranges

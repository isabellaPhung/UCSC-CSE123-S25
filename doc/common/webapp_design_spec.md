# Webpage Design Philosophy

The schedule companion requires a web application that will provide the user with a medium to manage user-to-device settings, manage user data, and view or edit scheduled items. This section of the document provides the principles that are necessary to the design of this webpage. These principles revolve around the categories defined in entry design and their acionable functionality within the user interface. It also includes necessary applications for the user interface such as the login, device management, and calendar pages. 


**Authentication and User Management:**
- User Authentication through login page
    - Sign up option within login page to register new user
        - create account with full name, username, email, and password
            - username uniqness check
            - password strength validation with visual feedback
            - automatic user ID generation 
    - forgot password option to reset credentials
    - prompts user for email and password 
        - credential validation against existing user
        - session token managment with CSRF protection
        - secure login state across browser session
        - includes persistant login state across browser sessions

**Device management**
- Device Registration
    - Add device with unique Device ID
        - Device will utilize wifi to link to Webserver
        - Option to link Device to user account
    - Switch between user devices 
    - Remove device from user directory
- Device State Persistence
    - Remember selected device across sessions
    - Handle device availability and connection status
    - User data linked synced to Device
    

**Main Page**
-
**Task Management**

**Habit Management**

**Event Management**

**Calendar**
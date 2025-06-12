## Server design

A web server will facilitate the communication between the device and the cloud backup, as well as serve a web app for data entry and viewing.
The server architecture is summarized in Figure \ref{manu_flow}.

![Example server architecture \label{manu_flow}](images/data_flow.png)

### Encryption

Security is a high priority when involving user data in a system involving the cloud.
To ensure security of the data being transmitted between the user and the device, the data must be encrypted using Transport Layer Security (TLS).
NGINX creates a secure connection by sending the ssl certificate to clients.
NGINX serves requests to the server coming from the client or the device, and routes it to the appropriate endpoint.

### Database Connection

User login and configuration information, as well as device backups are stored in a PostgreSQL database. 
This data storage is not directly accessible from the internet, and is accessed by making requests to the server API.
Devices making requests for database refreshes or data modification must make them to the "/api" url with an unexpired, valid API token.
The react front-end fetches user and device data from the database similarly to the device.

### Flask API

The Flask server has API endpoints which communicate with the database.
This includes endpoints to verify login credentials, get tasks/events/habits for a specific device, and update tasks/events/habits for a device.
The Flask server also has login and logout token endpoints to set and remove cookies containing JWT access tokens.
If an access token for a user is expired or not found, the Flask app will redirect the user to the login page. 
%% TODO elaborate more? mention javascript fetch is used to display user data on the web app
%% TODO device cookies, not just login

### Device security

When associating devices to accounts, it's important to ensure that users aren't able to access devices which aren't theirs.
In order to do this, each device will be shipped with a serial number and a code.
When adding a device, the user must enter the serial number and code, along with a name for the device.
%% TODO describe adding a device (with api token/security), accessing device data endpoint needs proper token

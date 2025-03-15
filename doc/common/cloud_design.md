![Overall structure of the device\label{emb_flow}](embedded_system_flow_chart.png){ width=70% }

The main goal of connecting the ESP32 to the cloud is to allow configuration from any device with access to a web browser, as well as allowing backups of the user's data.
The basic structure of the cloud is described in Figure \ref{emb_flow}

The ESP32 uses the `coreMQTT` library in order to make a connection with the AWS IoT core server.
The IoT server ensures a secure connection by using TLS / SSL, where the device is given a private key and a certificate to be used in verifying its identity.
Communication between the two endpoints will be done by publishing `json` packets.
The device receiving these messages can act accordingly by reading the message contents.
The IoT server can request data from the database service based on the message contents.

In order to achieve backups and other user data storage, the AWS Simple Storage Service (S3) will be used as an intermediary between the IoT server and the web sever. 
The user's schedule will be stored as an SQLite database in the same way that it is being stored on the device.
Whenever there are updates made to the database from the web server, the IoT core will send the changes over to the device when requested.

The web server will be using a `docker` instance with `nginx` and a `python` web server using the `Flask` and `boto3` packages to interface with the web requests and database respectively.
Using docker will simplify setup of the web server by creating a container for the programs to run inside.
This container will be deployed onto the Elastic Compute Cloud (EC2) service.

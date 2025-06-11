### Server {#proto_server}

Figure \ref{proto_flow} shows the structure of the cloud server during the prototyping stage.
The server consists of two parts: an MQTT endpoint that processes incoming and outbound messages,
and a web server that hosts the web app and processes any changes to the suer data.
Both components exchange data with the S3 data storage, where the user and device data exists.

![Connection between the prototype device and server \label{proto_flow}](images/prototype_data_flow.png)

The device connects to the AWS IoT MQTT endpoint and subscribes to the topic in the format of `iotdevice/<DEVICE ID>/datas3`.
Messages posted to the topic are routed based on the following rules:
`SELECT id, type FROM 'iotdevice/+/datas3' WHERE action='refresh' AND (type='task' OR type='habit' OR type='event')`
for requesting a refresh of the device's backup, and
`SELECT * FROM 'iotdevice/+/datas3' WHERE action = 'update'`
for requesting making changes to entries.
Each rule triggers a lambda function that retrieves the device's backup from the S3 device backups bucket and processes it as necessary.
The refresh retrieval function publishes each entry as a single message.
The update function reads the contents of the incoming message and makes changes to the entry of the same ID, then writes the new data back to S3.

The web server is hosted on a AWS EC2 instance deploying Caddy, gunicorn, and flask.
Caddy is a web server that automatically generates and renews TLS certificates.
Caddy communicates with the client through HTTPS, and forwards requests to the docker container that contains the gunicorn and flask sevrer.
The docker container is configured with a Dockerfile and credentials stored in a `.env` file.
Flask is used in conjunction with boto3 --- the python AWS SDK --- to serve the web app and to fetch user and device data.

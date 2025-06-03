## Server design

**Encryption**

Security is a high priority when involving user data in a system involving the cloud.
To ensure security of the data being transmitted from the user to the device and back, the data must be encypted using Transport Layer Security (TLS).
The prototype implements this by using
[Caddy](https://caddyserver.com/)
in its server software stack.
Caddy is a web server that automatically generates and renews TLS certificates. It also redirects any HTTP client traffic to HTTPS.
In our prototype, Caddy acts as an HTTPS reverse proxy, communicating with the client through HTTPS and with the Flask server through HTTP.
Ideally, NGINX should be used as the server, as it is more performant. %FIXME

![Connection between the prototype device and server](images/prototype_data_flow.png)

![Server architecture of the ideal product](images/data_flow.png)

**Database Connection**
The web server retrieves and updates the user’s data by accessing the cloud database.
In our prototype, this involves using Amazon’s boto3 Python toolkit to connect to our AWS S3 buckets.
All credentials and database configuration details are stored in a .env file, which is accessible by the Python code when the Docker container has been started.
Ideally, the cloud database should be a PostgreSQL instance running on a server. TODO: why postgres instead of s3
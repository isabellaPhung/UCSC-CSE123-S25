## Server design

**Encryption**

Security is a high priority when involving user data in a system involving the cloud.
To ensure security of the data being transmitted from the user to the device and back, the data must be encypted using Transport Layer Security (TLS).
The prototype implements this by using
[Caddy](https://caddyserver.com/)
in its server software stack.
Caddy is a web server that automatically generates and renews TLS certifiates.
Ideally, NGINX should be used as the server, as it is more performant. %FIXME

![Connection between the prototype device and server](prototype_data_flow.png)

![Server architecture of the ideal product](data_flow.png)

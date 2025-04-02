### Data cloud synchronization
Scope: cloud operations, security

Apparatus: productivity device, host machine for flashing and running tests, cloud server access

Independent variables: device, firmware

Dependent variables: security protocol, device certificate

Procedure:

1. Verify the device is connected to the internet.
2. Attempt connecting to the cloud endpoint using a url with `http://`
3. Ensure that the connection forces `https://`
4. Test adding a task.

Expectation: The connection must be made with ssl.

### Secure connection test report
\noindent
**Test Purpose**:
This is intended to verify that the device is capable of connecting to the internet and synchronizing with server information.

\noindent
**Test Procedure**:
Exact testing procedures can be found at \ref{security_procedure}. Results are visually observed.

\noindent
**Test Results**:

- Device had existing wifi credentials on it from \ref{wifi_test} so test is begun from step 2
- Using a laptop, a connection to the MQTT broker endpoint using 'http' was made
- The connection forced a https connection, ensuring the connection was secure

\noindent
**Final Analysis**:
The endpoint connection passes. The connection always forces https.

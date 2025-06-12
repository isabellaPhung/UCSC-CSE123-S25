### Software {#proto_sw}
The software for the ESP32-C3 was written in the C programming language, with the Espressif IoT Development Framework (esp-idf) for building, flashing, and testing the prototype.

The ESP32-C3 had three major roles to fill in our prototype: the graphical user interface (GUI), the database, and cloud synchronization. The GUI displays each respective type of entry within the database through a page system that the user can navigate using the directional buttons. The database maintains each entry on flash memory, manipulating entry data, and providing organized lists of entry data for the GUI to display. Cloud synchronization sends details about how the user modified their entries on the client, then receives a copy of all data within the server's database for the client's database to compare to.

The GUI is the most called system during runtime, which outputs to the device's monitor and inputs from the button interface on each cycle. Monitor communication is established through Serial Peripheral Interface (SPI) protocol, and rendered to using the Light and Versatile Graphics Library (LGVL). This graphics library provided a lightweight and fast C based GUI for display on the screen. Inputs where handled using the esp-iot button component, implementing our buttons for directional navigation, item selection, and on demand synchronization.

The database component serves as the persistent storage and retrieval system for all user-generated data, including tasks, events, habits, and habit entries. Implemented using SQLite3, it offers a lightweight yet robust solution well-suited for the restricted resources on the ESP32-C3, balancing performance with minimal resource overhead. To conserve memory, the system is designed to keep only the entries relevant to the current GUI screen in active memory. All long-term data is stored within flash memory through the database, allowing for efficient retrieval while maintaining a low runtime footprint. On screen changes, the GUI calls the database to retrieve the relevant entries for display. To improve modularity and reduce processing overhead on the interface side, these entries are returned in a pre-sorted state, ready for direct use by the frontend logic. Furthermore, when a task is marked as complete or deleted, the database is responsible for updating the underlying records accordingly, ensuring consistency between the visual representation and the stored data.

The prototype makes use of the coreMQTT and mbedTLS libraries to establish a secure connection to the MQTT broker. This connection is only established when the user presses the synchronization button, at which case the device will begin the multiple step process of synchronizing the user's data between the client and server. To prevent data races, the user's input is blocked and a loading popup will display on the monitor until all procedures are complete.

While the GUI and database components interact through random access memory, the cloud and database components interact through flash memory. Since the ESP32-C3 had limited remaining RAM while the MQTT broker was connected, we had to rely on our generous amount of flash memory at our disposal. We used an intermediary system, that retained the JSON messages and wrote the plaintext to a separate temporary file, as this would avoid the burden of having to deserialize the message and place it into the relational database while our resources were at their most stressed state. After accepting all messages from the broker, the client could then disconnect, freeing up enough resources for our database to then effectively parse all data left within the temporary file. Sending all server information to the client is the safest and simplest method to ensure the client and server display have identical data sets.

When alterations are made to an entry on the database a similar procedure is preformed. When a user preforms one of the several possible alterations to an entry or creates a new entry in the habit system, a new corresponding plaintext file is produced holding the relevant information about the change. Correlating each entry with its own file was intentionally preformed to avoid creating multiple tickets in case the user made several alterations to an entry's state before a cloud synchronization was preformed, in which case the alteration file would be continuously overwritten with the most recent change, effectively avoiding any redundant information from being parsed to the server. During cloud synchronization, each of these response files would be serialized into JSON and sent to the server, upon acknowledgement the file is then removed from flash memory. This would make sure that the client information is sent at least once. The server will compare the response with its own database, thus sending multiple alteration requests on the same entry will have no effect, and the client will be ensured that the server will have been given the same modification that the user made on the client.

This system of housing all data on the disk between cloud synchronization is highly robust, as all unfinished work will remain within non-volatile memory between power cycles. Furthermore, as our device must be capable of total offline use, all unsent alterations to the database will be preserved until broker connection could once again be secured, regardless of power cycling or later adjustments to the status of an entry by the user.

The device's Wi-Fi configuration is done through a HTTP server running on the device bound to its Wi-Fi access point.
The access point allows the user to connect to the device and access a landing page with a form to submit an SSID and password.
The device changes between the Wi-Fi connection modes
--- station only mode (sta), softAP only mode, and softAP + station mode (apsta) ---
based on its current state.
When the device boots, it checks the non-volatile storage (NVS) for an existing connection configuration.
Based on the results, it starts the ap or sta mode.
Figure \ref{wifi_prov_flow} shows the steps taken after the device boots.
Because the esp_wifi component is based on event callbacks, the provisioning process is able to run without blocking.
The HTTP server can be configured to either use a captive server, which redirects all HTTP requests to the login page, or with HTTPS.
Unfortunately these features are mutually exclusive,
tough a newer version of the esp-idf has introduced a feature that is able to do both at the same time with a different technology.

\input{wifi_provisioning}

The device is able to communicate with the server using coreMQTT and mbedTLS to establish a connection to the MQTT broker.
The main process must initialize the MQTT state, then follow these steps to communicate with the server:

1. Connect to the endpoint and clean any broken sessions that are present.
2. Subscribe to the device's topic and wait for a confirmation from the broker.
3. Publish the desired message to the topic. (Outlined in section \ref{device-server-communication})
4. Enter the process loop for a specified length of time.
5. Unsubscribe from the topic and wait for an acknowledgement.
6. Disconnect from the endpoint and close the connection.

coreMQTT is created to be used with an event callback structure, so any incoming messages must be handled in the callback as well.
In order to access the published payloads from outside of the library's functions,
a callback function type may be given to the initialization function.
This callback function is defined in the app_main.c file, and is triggered when any message is published on the device's topic.
The payload is then saved on disk to be processed later.

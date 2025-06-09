## Gantt chart
The overall plan for the project was managed with a gantt chart.
Issue metadata was used to generate the chart when there were updates to the plan.

![The gannt chart at the end of the prototype development](gantt.png)

## Division of Labor
Isabella, Lennan, and Mason worked on writing software to run on the hardware.
Akanksha, Lennan, and Sulaiman worked on the cloud computing software bring up.

Major tasks involved: 

- Hardware
  - Creating the database manager component
  - Creating the user interface component 
  - Creating the Wi-Fi provisioning component
  - Creating the MQTT communication component
  - Connecting components to communicate with each other in a main loop
- Cloud
  - Configuring the virtual server
    - Configuring the server and api reverse proxy
    - Configuring the DNS forwarding service
  - Creating the web application back-end
  - Creating the web app front-end and its UI
  - Configuring the bridge between the device and the data-storage
    - Define the packet structure, as well as its storage and transmission

## Collaboration
The prototype development and documentation was organized on the UCSC Gitlab instance.
Tasks were assigned by creating an issue and ranked on the issue board by its current status.
Open issues were for tasks that were being considered as necessary.
Stretch goals started here, and went to closed if they were deemed unnecessary / out of time.
The issue gets tagged *pending* if it was required for making progress.
Once there was a team member actively working on that issue, they would self-assign and tag it as *in-progress*.
Once resolved, it would be moved to the *closed* status.
If it needed more work, it would be moved back to the *pending* status.
A minimal amount of work was done on the Google Workspace platform.

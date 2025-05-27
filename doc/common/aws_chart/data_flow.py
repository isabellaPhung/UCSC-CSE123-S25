from diagrams import Diagram, Cluster, Edge

from diagrams.aws.compute import EC2, Lambda
from diagrams.aws.storage import S3
from diagrams.aws.iot import IotCore
from diagrams.aws.network import Route53

from diagrams.programming.framework import Flask
from diagrams.programming.language import Python

from diagrams.onprem.network import Caddy, Gunicorn
from diagrams.onprem.container import Docker

from diagrams.generic.device import Tablet

from diagrams.custom import Custom

font = {
    "fontname": "Roboto",
    "labelfontname": "Roboto",
    "dpi": "300"
}

with Diagram("Data Flow", show=False, graph_attr=font, node_attr=font, edge_attr=font):
    esp32 = Custom("ESP32", "./custom/esp32.png")
    tablet = Tablet("Web app")

    with Cluster("AWS Cloud", graph_attr=font):
        s3 = S3("Data Storage")
        dns = Route53("DNS Service")
        completion = Lambda("Modify Data")
        iotcore = IotCore("Receive Message")

        with Cluster("EC2 Instance", graph_attr=font):
                caddy = Caddy("Reverse Proxy")

                with Cluster("Docker Container", graph_attr=font):
                    gunicorn = Gunicorn("WSGI Server")
                    flask = Flask("Framework")
        
    s3 << [flask, completion]
    tablet >> Edge(label="HTTPS") >> dns >> caddy >> gunicorn >> flask
    completion >> Edge(label="Publish") >>  iotcore
    completion << Edge(label="Trigger") << iotcore
    iotcore >> Edge(label="MQTT") <<  esp32

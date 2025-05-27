from diagrams import Diagram, Cluster, Edge

from diagrams.aws.compute import EC2
from diagrams.aws.iot import IotGeneric
from diagrams.aws.network import Route53
from diagrams.aws.general import User

from diagrams.programming.framework import Flask, React

from diagrams.onprem.network import Nginx
from diagrams.onprem.database import Postgresql

from diagrams.generic.os import Android

font = {
    "fontname": "Roboto",
    "labelfontname": "Roboto",
    "dpi": "300"
}

with Diagram("", show=False, graph_attr=font, node_attr=font, edge_attr=font):
    device = IotGeneric("IoT device")
    android = Android("Mobile app")

    with Cluster("AWS Cloud", graph_attr=font):
        dns = Route53("DNS Service")

        with Cluster("EC2 Instance", graph_attr=font):
            with Cluster("React Docker Image", graph_attr=font):
                nginx = Nginx("NGINX server")
                react = React("Web Application")
            with Cluster("Flask Docker Image", graph_attr=font):
                flask = Flask("Web Framework")
            postgres = Postgresql("User + Device Data")

    android >> dns
    device >> dns
    dns >> nginx
    nginx >> Edge(label="/api") >> flask
    nginx >> Edge(label="localhost") >> react
    flask >> postgres

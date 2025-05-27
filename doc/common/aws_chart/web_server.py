from diagrams import Diagram, Cluster, Edge

from diagrams.aws.compute import EC2
from diagrams.aws.storage import S3

from diagrams.programming.framework import Flask
from diagrams.programming.language import Python

from diagrams.onprem.network import Caddy, Gunicorn
from diagrams.onprem.container import Docker

from diagrams.generic.device import Tablet

with Diagram("Web Server", show=False):
    s3 = S3("User and Device Data")
    tablet = Tablet("User")

    with Cluster(""):
        ec2 = EC2("EC2 Instance")
        caddy = Caddy("Reverse proxy")

        tablet >> Edge() << caddy

        with Cluster(""):
            docker = Docker("Docker Container")
            gunicorn = Gunicorn("Server")
            flask = Flask("Framework")
            boto3 = Python("AWS SDK boto3")
            caddy >> Edge() << gunicorn
            gunicorn >> Edge() << flask
            flask >> Edge() << boto3
            boto3 >> Edge() << s3

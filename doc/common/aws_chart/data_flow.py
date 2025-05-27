from diagrams import Diagram, Cluster, Edge

from diagrams.aws.compute import EC2, Lambda
from diagrams.aws.storage import S3
from diagrams.aws.iot import IotCore

from diagrams.generic.device import Tablet

from diagrams.custom import Custom

with Diagram("Data Flow", show=False):
    esp32 = Custom("ESP32", "./custom/esp32.png")

    tablet = Tablet("Web app")

    with Cluster("AWS Cloud"):
        s3 = S3("Data Storage")
        ec2 = EC2("Web Server")

        completion = Lambda("Modify Data")
        request = Lambda("Request Data")

        iotcore = IotCore("MQTT Broker")


    esp32 >> Edge() << iotcore

    iotcore >> request >> s3
    iotcore << request << s3

    iotcore >> completion >> s3
    iotcore << completion

    s3 >> Edge() << ec2

    ec2 >> Edge() << tablet

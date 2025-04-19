import json
import boto3


class AwsS3:
    def __init__(self):
        self.s3 = boto3.resource('s3')
        self.obj = self.s3.Object("iot-device-data-backups", "55/task0.json")
        self.data = json.loads(self.obj.get()["Body"].read().decode('utf-8'))

    def get_tasks(self):
        return self.data

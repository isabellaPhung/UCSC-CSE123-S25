import boto3
import json

def lambda_handler(event, context):

    # todo get dvice id from event
    client = boto3.client('iot-data')
    s3 = boto3.resource('s3')
    obj = s3.Object("iot-device-data-backups", "55/task0.json")
    data = json.loads(obj.get()["Body"].read().decode('utf-8'))

    response = client.publish(
        topic='iotdevice/55/datas3',
        qos=1,
        payload=json.dumps(data)
    )
    return 0

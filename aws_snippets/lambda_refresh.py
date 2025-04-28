import boto3
import json

def lambda_handler(event, context):

    # todo get dvice id from event
    client = boto3.client('iot-data')#, region_name='us-west-1')
    s3 = boto3.resource('s3')
    obj = s3.Object("iot-device-data-backups", "55/task0.json")
    data = json.loads(obj.get()["Body"].read().decode('utf-8'))

    initial_response = {
        "id" : "server",
        "action" : "length",
        "length" : len(data["tasks"])
    }
    response = client.publish(
            topic='iotdevice/55/datas3',
            qos=1,
            payload=json.dumps(initial_response)
    )

    index = 0
    for task in data["tasks"]:
        response_data = {
            "id" : "server",
            "action" : "response",
            "index" : index,
            "task" : task
        }

        response = client.publish(
            topic='iotdevice/55/datas3',
            qos=1,
            payload=json.dumps(response_data)
        )
        index += 1
    return 0

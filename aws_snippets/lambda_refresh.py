import boto3
import json

client = boto3.client("iot-data")#, region_name='us-west-1')
s3 = boto3.resource("s3")

def lambda_handler(event, context):
    device_id = event["id"]
    data_type = event["type"]
    obj = s3.Object("iot-device-data-backups", device_id + "/" + data_type + "0.json")
    data = json.loads(obj.get()["Body"].read().decode("utf-8"))
    arr = data[data_type]
    length = len(arr)

    initial_response = {
        "id" : "server",
        "action" : "length",
        "length" : length
    }
    response = client.publish(
            topic="iotdevice/" + device_id + "/datas3",
            qos=1,
            payload=json.dumps(initial_response)
    )

    index = 0
    for data_pub in arr:
        response_data = {
            "id" : "server",
            "action" : "response",
            "index" : index,
            data_type : data_pub
        }

        response = client.publish(
            topic="iotdevice/" + device_id + "/datas3",
            qos=1,
            payload=json.dumps(response_data)
        )
        index += 1
    return 200

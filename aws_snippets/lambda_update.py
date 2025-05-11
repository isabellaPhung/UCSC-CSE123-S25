import json
import boto3

client = boto3.client("iot-data")#, region_name='us-west-1')
s3 = boto3.resource('s3')

def lambda_handler(event, context):
    device_id = event["id"]
    obj = s3.Object("iot-device-data-backups", device_id + "/task0.json")
    data = json.loads(obj.get()["Body"].read().decode("utf-8"))
    arr = data["task"]

    for task in event["task"]:
        # if you don't want to not allow the device to un-complete an event
        #if task["completion"] == 1:
        #    continue
        uuid = task["id"]
        for task_db in data["task"]:
            # do not allow modifying a deleted task
            if task_db["id"] == uuid and task_db["completion"] != 0:
                task_db["completion"] = task["completion"]

    obj.put(
        Body = (bytes(json.dumps(data, indent = 2).encode('UTF-8'))),
        ContentType = "application/json"
    )

    ack = {
        "id" : "server",
        "action" : "ack"
    }
    response = client.publish(
            topic="iotdevice/" + device_id + "/datas3",
            qos=1,
            payload=json.dumps(ack)
    )

    return 0

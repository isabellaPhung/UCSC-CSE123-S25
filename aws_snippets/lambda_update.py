import json
import boto3

def lambda_handler(event, context):
    s3 = boto3.resource('s3')
    obj = s3.Object("iot-device-data-backups", "55/task0.json")
    data = json.loads(obj.get()["Body"].read().decode('utf-8'))

    for task in event["tasks"]:
        # if you don't want to not allow the device to un-complete an event
        #if task["completion"] == 1:
        #    continue
        uuid = task["id"]
        for task_db in data["tasks"]:
            # do not allow modifying a deleted task
            if task_db["id"] == uuid and task_db["completion"] != 0:
                task_db["completion"] = task["completion"]

    obj.put(
        Body = (bytes(json.dumps(data, indent = 2).encode('UTF-8'))),
        ContentType = "application/json"
    )

    return 0

import json
import boto3

def lambda_handler(event, context):
    s3 = boto3.resource('s3')
    obj = s3.Object("iot-device-data-backups", "55/task0.json")
    data = json.loads(obj.get()["Body"].read().decode('utf-8'))

    for task in event["tasks"]:
        if task["completion"] == "incomplete":
            continue
        uuid = task["id"]
        for task_db in data["tasks"]:
            if task_db["id"] == uuid:
                task_db["completion"] = task["completion"]

    obj.put(
        Body = (bytes(json.dumps(data, indent = 2).encode('UTF-8'))),
        ContentType = "application/json"
    )

    return 0

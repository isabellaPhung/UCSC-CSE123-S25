import json
import boto3

client = boto3.client("iot-data")#, region_name='us-west-1')
s3 = boto3.resource('s3')

def lambda_handler(event, context):
    device_id = event["id"]
    data_type = event["type"]
    obj = s3.Object("iot-device-data-backups", device_id + "/" + data_type + "0.json")
    data = json.loads(obj.get()["Body"].read().decode("utf-8"))
    arr_s3 = data[data_type]

    for entry in event[data_type]:
        skip = False
        uuid = entry["id"]
        for entry_s3 in arr_s3:
            if entry_s3["id"] == uuid:
                if data_type == "task" and entry_s3["completion"] != 2:
                    # do not allow modifying a deleted task
                    entry_s3["completion"] = entry["completion"]
                elif data_type == "habit":
                    for dates in entry_s3["completed"]:
                        if dates == entry["date"]:
                            skip = True
                    if not skip:
                        entry_s3["completed"].append(entry["date"])
                elif data_type == "event":
                    entry_s3["deleted"] = 1

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

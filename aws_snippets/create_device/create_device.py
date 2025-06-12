import boto3
import sys
import json

if __name__ == '__main__':
    argc = len(sys.argv)
    if argc < 4:
        print(f"Usage: {sys.argv[0]} <access key> <secret key> <device id>")
        exit()

    access_key = sys.argv[1]
    secret_key = sys.argv[2]
    device_id = sys.argv[3]

    s3 = boto3.resource('s3',
            aws_access_key_id=access_key,
            aws_secret_access_key=secret_key,
            )

    skip = False
    obj = s3.Object("user-device-credentials", "devices.json")
    data = json.loads(obj.get()["Body"].read().decode("utf-8"))
    for devices in data["devices"]:
        if devices["id"] == device_id:
            skip = True
    if not skip:
        device_data = json.loads(f"{{\"id\":\"{device_id}\"}}")
        data["devices"].append(device_data)

        obj.put(
            Body = (bytes(json.dumps(data, indent = 2).encode('UTF-8'))),
            ContentType = "application/json"
        )

    data_types = ["task", "event", "habit"]
    for i in range(3):
        blank_json = json.loads(f"{{\"{data_types[i]}\":[]}}")

        obj = s3.Object("iot-device-data-backups", device_id + "/" + data_types[i] + "0.json")
        obj.put(
            Body = (bytes(json.dumps(blank_json, indent = 2).encode("utf-8"))),
            ContentType = "application/json"
        )

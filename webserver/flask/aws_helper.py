import json
import os
import uuid
import boto3


class AwsS3:
    def __init__(self):
        self.s3 = boto3.resource('s3')
        self.device_bucket = os.getenv("DEVICE_DATA_BUCKET")
        self.user_bucket = os.getenv("USER_DATA_BUCKET")

    def get_tasks(self):
        obj = self.s3.Object(self.device_bucket, "55/task0.json")
        data = json.loads(obj.get()["Body"].read().decode('utf-8'))
        return data["tasks"]

    def check_login(self, username, password):
        obj = self.s3.Object(self.user_bucket, "users.json")
        data = json.loads(obj.get()["Body"].read().decode('utf-8'))

        for user in data["users"]:
            if user["username"] == username:
                if user["password"] == password:
                    return True
                return False
        return False

    def add_user(self, fullname, username, password):
        obj = self.s3.Object(self.user_bucket, "users.json")
        data = json.loads(obj.get()["Body"].read().decode("utf-8"))

        for user in data["users"]:
            if user["username"] == username:
                return False

        # TODO: devices?
        data["users"].append({"fullname": fullname, "username": username, "password": password})

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True

    def add_task(self, name, description, duedate):
        obj = self.s3.Object(self.device_bucket, "55/task0.json")
        data = json.loads(obj.get()["Body"].read().decode("utf-8"))

        # TODO: check default values
        id = str(uuid.uuid4())
        completion = 0
        priority = 1

        data["tasks"].append(
            {"id": id, "name": name, "description": description,
             "completion": completion, "priority": priority, "duedate": duedate}
        )

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True

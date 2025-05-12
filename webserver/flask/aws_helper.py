import json
import os
import uuid
import boto3


class AwsS3:
    def __init__(self):
        self.s3 = boto3.resource('s3')
        self.device_bucket = os.getenv("DEVICE_DATA_BUCKET")
        self.user_bucket = os.getenv("USER_DATA_BUCKET")
        self.user_file = os.getenv("USER_FILE")
        self.task_file = os.getenv("TASK_FILE")
        self.event_file = os.getenv("EVENT_FILE")
        self.habit_file = os.getenv("HABIT_FILE")

    def load_info(self, bucket, data_only=False):
        device_id = "55"

        if bucket == "user":
            obj = self.s3.Object(self.user_bucket, self.user_file)
        elif bucket == "task":
            obj = self.s3.Object(self.device_bucket, f"{device_id}/{self.task_file}")
        elif bucket == "event":
            obj = self.s3.Object(self.device_bucket, f"{device_id}/{self.event_file}")
        elif bucket == "habit":
            obj = self.s3.Object(self.device_bucket, f"{device_id}/{self.habit_file}")
        else:
            return False

        data = json.loads(obj.get()["Body"].read().decode('utf-8'))

        if data_only:
            return data
        return obj, data

    def check_login(self, username, password):
        obj, data = self.load_info("user")

        for user in data["users"]:
            if user["username"] == username:
                if user["password"] == password:
                    return True
                return False
        return False

    def add_user(self, fullname, username, password):
        obj, data = self.load_info("user")

        for user in data["users"]:
            if user["username"] == username:
                return False

        data["users"].append({"fullname": fullname, "username": username, "password": password, "devices": []})

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True

    def delete_user(self, username):
        obj, data = self.load_info("user")

        data["users"] = [user for user in data["users"] if user["username"] != username]

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True

    def add_task(self, name, description, completion, priority, duedate):
        obj, data = self.load_info("task")

        id = str(uuid.uuid4())

        data["task"].append(
            {"id": id, "name": name, "description": description,
             "completion": completion, "priority": priority, "duedate": duedate}
        )

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True

    def get_tasks(self, start_timestamp, end_timestamp):
        obj, data = self.load_info("task")

        data["task"] = [task for task in data["task"]
                        if start_timestamp <= task["duedate"] <= end_timestamp]

        return data

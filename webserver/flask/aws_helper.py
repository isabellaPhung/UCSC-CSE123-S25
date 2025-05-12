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

    def load_info(self, elem):
        device_id = "55"

        if elem == "task":
            obj = self.s3.Object(self.device_bucket, f"{device_id}/{self.task_file}")
        elif elem == "event":
            obj = self.s3.Object(self.device_bucket, f"{device_id}/{self.event_file}")
        elif elem == "habit":
            obj = self.s3.Object(self.device_bucket, f"{device_id}/{self.habit_file}")
        else:
            return False

        data = json.loads(obj.get()["Body"].read().decode('utf-8'))
        return obj, data

    def get_users(self, data_only=False):
        obj = self.s3.Object(self.user_bucket, self.user_file)
        data = json.loads(obj.get()["Body"].read().decode('utf-8'))

        if data_only:
            return data
        return obj, data

    def check_login(self, username, password):
        data = self.get_users(data_only=True)

        for user in data["users"]:
            if user["username"] == username:
                if user["password"] == password:
                    return True
                return False
        return False

    def add_user(self, fullname, username, password):
        obj, data = self.get_users()

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
        obj, data = self.get_users()

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

    def add_event(self, name, starttime, duration):
        obj, data = self.load_info("event")

        id = str(uuid.uuid4())

        data["event"].append(
            {"id": id, "name": name, "starttime": starttime, "duration": duration}
        )

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True

    def get_events(self, start_timestamp, end_timestamp):
        obj, data = self.load_info("event")

        data["event"] = [event for event in data["event"]
                         if start_timestamp <= event["starttime"] <= end_timestamp]

        return data

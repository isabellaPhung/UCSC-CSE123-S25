from datetime import datetime, timedelta
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
        self.device_file = os.getenv("DEVICE_FILE")
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

    def get_user_devices(self, username):
        obj, data = self.get_users()

        for user in data["users"]:
            if user["username"] == username:
                return {"devices": user["devices"]}

    def get_all_devices(self):
        obj = self.s3.Object(self.user_bucket, self.device_file)
        data = json.loads(obj.get()["Body"].read().decode('utf-8'))

        devices = [device["id"] for device in data["devices"]]

        return devices

    def add_device(self, username, device_id, name):
        if device_id not in self.get_all_devices():
            return False, "Device ID not found"

        obj, data = self.get_users()

        for user in data["users"]:
            if user["username"] == username:
                if device_id in [device["id"] for device in user["devices"]]:
                    return False, "Device already added"
                user["devices"].append({"id": device_id, "name": name})

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True, "Success"

    def delete_device(self, username, device_id):
        obj, data = self.get_users()

        for user in data["users"]:
            if user["username"] == username:
                user["devices"] = [device for device in user["devices"]
                                   if device["id"] != device_id]

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

        today_data = [task for task in data["task"]
                      if start_timestamp <= task["duedate"] <= end_timestamp]

        data["task"] = sorted(today_data, key=lambda task: task["duedate"])

        return data

    def get_all_tasks(self):
        obj, data = self.load_info("task")
        return data

    def add_event(self, name, description, starttime, duration):
        obj, data = self.load_info("event")

        id = str(uuid.uuid4())

        data["event"].append(
            {"id": id, "name": name, "description": description,
             "starttime": starttime, "duration": duration}
        )

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True

    def delete_event(self, id):
        obj, data = self.load_info("event")

        data["event"] = [event for event in data["event"] if event["id"] != id]

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True

    def get_events(self, start_timestamp, end_timestamp):
        obj, data = self.load_info("event")

        today_data = [event for event in data["event"]
                      if start_timestamp <= event["starttime"] <= end_timestamp]

        data["event"] = sorted(today_data, key=lambda event: event["starttime"])

        return data

    def add_habit(self, name, goal):
        obj, data = self.load_info("habit")

        id = str(uuid.uuid4())

        data["habit"].append(
            {"id": id, "name": name, "goal": goal, "completed": []}
        )

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True

    def update_habit(self, id, current_date, completed):
        obj, data = self.load_info("habit")

        for habit in data["habit"]:
            if habit["id"] == id:
                if completed:
                    habit["completed"].append(current_date)
                else:
                    habit["completed"].remove(current_date)

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True

    def delete_habit(self, id):
        obj, data = self.load_info("habit")

        data["habit"] = [habit for habit in data["habit"] if habit["id"] != id]

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True

    def get_habits(self, current_date):
        obj, data = self.load_info("habit")

        today = datetime.strptime(current_date, "%Y-%m-%d")
        monday = today - timedelta(days=today.weekday())
        week = [(monday + timedelta(days=i)).strftime("%Y-%m-%d") for i in range(7)]

        for habit in data["habit"]:
            habit["completed"] = [date for date in habit["completed"] if date in week]
            # For web app checkboxes
            habit["days"] = [day in habit["completed"] for day in week]

        return data

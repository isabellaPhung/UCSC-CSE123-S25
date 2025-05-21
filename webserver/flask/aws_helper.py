from cryptography.fernet import Fernet
from datetime import datetime
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
        self.secret_key = os.getenv("COOKIE_SECRET_KEY")
        self.fernet = Fernet(self.secret_key)

    def encrypt_id(self, username, device_id):
        """
        Encrypt a user's device ID

        Args:
            username (str): username of account
            device_id (str): ID of selected device

        Returns:
            str: encrypted string containing username, device_id
        """
        user = {"username": username, "device_id": device_id}
        return self.fernet.encrypt(json.dumps(user).encode()).decode()

    def decrypt_id(self, encrypted_data):
        """
        Decrypt a user's device ID

        Args:
            encrypted_data (str): encrypted string containing username, device_id

        Returns:
            str: ID of device contained in encrypted string
        """
        user = json.loads(self.fernet.decrypt(encrypted_data.encode()).decode())
        return user["device_id"]

    def load_info(self, elem, device_id):
        """
        Get element info for a specific device

        Args:
            elem (str): string representing "task", "event", or "habit"
            device_id (str): ID of selected device

        Returns:
            s3.Object, json: s3 object and data from loaded file
        """
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
        """
        Get data for all users

        Args:
            data_only (bool): return only data if True

        Returns:
            s3.Object, json: s3 object and data from users file
        """
        obj = self.s3.Object(self.user_bucket, self.user_file)
        data = json.loads(obj.get()["Body"].read().decode('utf-8'))

        if data_only:
            return data
        return obj, data

    def check_login(self, username, password):
        """
        Check provided username and password against database

        Args:
            username (str): username of user account
            password (str): password of user account

        Returns:
            bool: True if username and password match, False otherwise
        """
        data = self.get_users(data_only=True)

        for user in data["users"]:
            if user["username"] == username:
                if user["password"] == password:
                    return True
                return False
        return False

    def add_user(self, fullname, username, password):
        """
        Add user to user database file

        Args:
            fullname (str): full name of user account
            username (str): username of user account
            password (str): password of user account

        Returns:
            bool: True if user account is successfully added, False otherwise
        """
        obj, data = self.get_users()

        # If username already exists, return False
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
        """
        Delete user from user database file

        Args:
            username (str): username of user account

        Returns:
            bool: True
        """
        obj, data = self.get_users()

        data["users"] = [user for user in data["users"] if user["username"] != username]

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True

    def get_user_devices(self, username):
        """
        Get all devices added to a user account

        Args:
            username (str): username of user account

        Returns:
            json: key = "devices", value = list of devices
        """
        obj, data = self.get_users()

        for user in data["users"]:
            if user["username"] == username:
                return {"devices": user["devices"]}

    def get_all_devices(self):
        """
        Get IDs of all manufactured devices

        Returns:
            list: IDs of all manufactured devices
        """
        obj = self.s3.Object(self.user_bucket, self.device_file)
        data = json.loads(obj.get()["Body"].read().decode('utf-8'))

        devices = [device["id"] for device in data["devices"]]

        return devices

    def add_device(self, username, device_id, name):
        """
        Add device to a user account

        Args:
            username (str): username of user account
            device_id (str): ID of device
            name (str): name of device

        Returns:
            bool, str: True + "Success" if successfully added, False + msg otherwise
        """
        if device_id not in self.get_all_devices():
            return False, "Device ID not found"

        obj, data = self.get_users()

        for user in data["users"]:
            if user["username"] == username:
                if device_id in [device["id"] for device in user["devices"]]:
                    return False, "Device already added"
                user["devices"].append({"id": device_id, "name": name})
                break

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True, "Success"

    def update_device(self, username, device_id, name):
        """
        Update the details of a device

        Args:
            username (str): username of user account
            device_id (str): ID of device
            name (str): name of device

        Returns:
            bool, str: True + "Success" if successfully updated, False + msg otherwise
        """
        if device_id not in self.get_all_devices():
            return False, "Device ID not found"

        obj, data = self.get_users()

        found = False
        for user in data["users"]:
            if user["username"] == username:
                for device in user["devices"]:
                    if device["id"] == device_id:
                        found = True
                        device["name"] = name
                        break
                if not found:
                    user["devices"].append({"id": device_id, "name": name})

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True, "Success"

    def delete_device(self, username, device_id):
        """
        Delete device from a user account

        Args:
            username (str): username of user account
            device_id (str): ID of device

        Returns:
            bool: True
        """
        obj, data = self.get_users()

        for user in data["users"]:
            if user["username"] == username:
                user["devices"] = [device for device in user["devices"]
                                   if device["id"] != device_id]
                break

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True

    def active_device(self, username, encrypted_id):
        """
        Add device to a user account

        Args:
            username (str): username of user account
            device_id (str): ID of added device
            encrypted_id (str): encrypted string containing username, device_id

        Returns:
            bool, str: True + "Success" if successfully added, False + msg otherwise
        """
        data = self.get_user_devices(username)
        id = self.decrypt_id(encrypted_id)

        for device in data["devices"]:
            if device["id"] == id:
                return device
        return False

    def add_task(self, name, description, completion, priority, duedate, encrypted_id):
        """
        Add a task

        Args:
            name (str): name of task
            description (str): description of task
            completion (int): completion level of task (default: 0)
            priority (int): priority level of task (1 - 3)
            duedate (int): due date of task (UTC timestamp)
            encrypted_id (str): encrypted string containing username, device_id

        Returns:
            bool: True
        """
        device_id = self.decrypt_id(encrypted_id)
        obj, data = self.load_info("task", device_id)

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

    def update_task(self, id, completion, encrypted_id):
        """
        Update a task's completion status

        Args:
            id (str): Task ID
            completion (int): Task completion status (0=deleted, 1=incomplete, 2=complete)

        Returns:
            bool: True if successful, False otherwise
        """
        device_id = self.decrypt_id(encrypted_id)
        obj, data = self.load_info("task", device_id)

        # Find and update the task
        found = False
        for task in data["task"]:
            if task["id"] == id:
                task["completion"] = completion
                found = True
                break

        if not found:
            return False

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True

    def get_tasks(self, start_timestamp, end_timestamp, encrypted_id):
        """
        Get today's tasks for a user account

        Args:
            start_timestamp (str): timestamp for start of day (UTC timestamp)
            end_timestamp (str): timestamp for end of day (UTC timestamp)
            encrypted_id (str): encrypted string containing username, device_id

        Returns:
            json: key = "task", value = list of tasks sorted by due date
        """
        device_id = self.decrypt_id(encrypted_id)
        obj, data = self.load_info("task", device_id)

        # Get tasks due today that have not been deleted
        today_data = [task for task in data["task"]
                      if start_timestamp <= task["duedate"] <= end_timestamp
                      and task["completion"] != 2]

        data["task"] = sorted(today_data, key=lambda task: task["duedate"])

        return data

    def get_all_tasks(self, encrypted_id):
        """
        Get all tasks from a user account

        Args:
            encrypted_id (str): encrypted string containing username, device_id

        Returns:
            json: key = "task", value = list of tasks
        """
        device_id = self.decrypt_id(encrypted_id)
        obj, data = self.load_info("task", device_id)

        # Filter out tasks marked as deleted
        data["task"] = [task for task in data["task"] if task["completion"] != 2]
        return data

    def add_event(self, name, description, starttime, duration, deleted, encrypted_id):
        """
        Add an event

        Args:
            name (str): name of event
            description (str): description of event
            starttime (int): timestamp of event start time (UTC timestamp)
            duration (int): duration of event (seconds)

            encrypted_id (str): encrypted string containing username, device_id

        Returns:
            bool: True
        """
        device_id = self.decrypt_id(encrypted_id)
        obj, data = self.load_info("event", device_id)

        id = str(uuid.uuid4())

        data["event"].append(
            {"id": id, "name": name, "description": description,
             "starttime": starttime, "duration": duration, "deleted": deleted}
        )

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True

    def delete_event(self, id, encrypted_id):
        """
        Delete an event

        Args:
            id (str): uuid of event
            encrypted_id (str): encrypted string containing username, device_id

        Returns:
            bool: True
        """
        device_id = self.decrypt_id(encrypted_id)
        obj, data = self.load_info("event", device_id)

        for event in data["event"]:
            if event["id"] == id:
                event["deleted"] = 1

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True

    def get_events(self, start_timestamp, end_timestamp, encrypted_id):
        """
        Get events for a certain day from a user account

        Args:
            start_timestamp (int): timestamp for start of day (UTC timestamp)
            end_timestamp (int): timestamp for end of day (UTC timestamp)
            encrypted_id (str): encrypted string containing username, device_id

        Returns:
            json: key = "event", value = list of events sorted by start time
        """
        device_id = self.decrypt_id(encrypted_id)
        obj, data = self.load_info("event", device_id)

        # Get events due today that have not been deleted
        today_data = [event for event in data["event"]
                      if start_timestamp <= event["starttime"] <= end_timestamp
                      and event["deleted"] != 1]

        data["event"] = sorted(today_data, key=lambda event: event["starttime"])

        return data

    def add_habit(self, name, goal, encrypted_id):
        """
        Add a habit

        Args:
            name (str): name of habit
            goal (int): integer representation of scheduled days for habit
            encrypted_id (str): encrypted string containing username, device_id

        Returns:
            bool: True
        """
        device_id = self.decrypt_id(encrypted_id)
        obj, data = self.load_info("habit", device_id)

        id = str(uuid.uuid4())

        data["habit"].append(
            {"id": id, "name": name, "goal": goal, "completed": []}
        )

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True

    def update_habit(self, id, timestamp, completed, start_timestamp, end_timestamp, encrypted_id):
        """
        Update a habit

        Args:
            id (str): uuid of habit
            timestamp (int): timestamp of day to update habit (UTC timestamp)
            completed (bool): whether habit was marked completed or uncompleted
            start_timestamp (int): timestamp for start of day (UTC timestamp)
            end_timestamp (int): timestamp for end of day (UTC timestamp)
            encrypted_id (str): encrypted string containing username, device_id

        Returns:
            bool: True
        """
        device_id = self.decrypt_id(encrypted_id)
        obj, data = self.load_info("habit", device_id)

        for habit in data["habit"]:
            if habit["id"] == id:
                if completed:
                    habit["completed"].append(timestamp)
                else:
                    habit["completed"] = [date for date in habit["completed"]
                                          if date < start_timestamp or date > end_timestamp]
                break

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True

    def delete_habit(self, id, encrypted_id):
        """
        Delete a habit

        Args:
            id (str): uuid of habit
            encrypted_id (str): encrypted string containing username, device_id

        Returns:
            bool: True
        """
        device_id = self.decrypt_id(encrypted_id)
        obj, data = self.load_info("habit", device_id)

        data["habit"] = [habit for habit in data["habit"] if habit["id"] != id]

        obj.put(
            Body=(bytes(json.dumps(data, indent=2).encode("utf-8"))),
            ContentType="application/json"
        )
        return True

    def get_habits(self, current_date, start_timestamp, encrypted_id):
        """
        Get habits for the current week

        Args:
            current_date (str): string representation of today's date (y-m-d)
            start_timestamp (int): timestamp for start of today (UTC timestamp)
            encrypted_id (str): encrypted string containing username, device_id

        Returns:
            bool: True
        """
        device_id = self.decrypt_id(encrypted_id)
        obj, data = self.load_info("habit", device_id)

        today = datetime.strptime(current_date, "%Y-%m-%d")
        # Monday = 0, ..., Sunday = 6
        offset = today.weekday()
        # Calculate UTC timestamp for Monday @ 12:00 AM
        monday_start = start_timestamp - offset * 86400
        # Calculate UTC timestamp for Sunday @ 11:59 PM
        sunday_end = monday_start + 7 * 86400 - 1

        # Save start and end timestamps for each day this week
        week = [(monday_start + i * 86400, monday_start + 86399 + i * 86400)
                for i in range(7)]

        for habit in data["habit"]:
            # To use for web UI checkboxes
            habit["days"] = [False] * 7
            for timestamp in habit["completed"]:
                # Check if the timestamp falls during this week
                if monday_start <= timestamp <= sunday_end:
                    for i in range(7):
                        # Mark the specific day it occurs as True
                        if week[i][0] <= timestamp <= week[i][1]:
                            habit["days"][i] = True
                            break
        return data

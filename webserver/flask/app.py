import os
from datetime import datetime, timedelta, timezone
from flask import Flask, request, render_template, redirect, url_for, jsonify
from flask_jwt_extended import (
    JWTManager, create_access_token, create_refresh_token, unset_jwt_cookies,
    set_access_cookies, set_refresh_cookies, get_jwt, get_jwt_identity, jwt_required
)
from aws_helper import AwsS3

app = Flask(__name__)
s3_conn = AwsS3()

app.config['JWT_COOKIE_SECURE'] = True
app.config['JWT_COOKIE_CSRF_PROTECT'] = True
app.config["JWT_SECRET_KEY"] = os.getenv("JWT_SECRET_KEY")
app.config["JWT_TOKEN_LOCATION"] = [os.getenv("JWT_TOKEN_LOCATION")]
app.config["JWT_ACCESS_TOKEN_EXPIRES"] = int(os.getenv("JWT_ACCESS_TOKEN_EXPIRES"))
# app.config["JWT_REFRESH_COOKIE_PATH"] = "/token/refresh"

jwt = JWTManager(app)


@app.route("/test")
def test():
    device_id = request.cookies.get("device_id")
    return s3_conn.active_device("test", device_id), 200


@app.route("/token/login", methods=["POST"])
def login_authenticate():
    username = request.json.get("username")
    password = request.json.get("password")

    if not s3_conn.check_login(username, password):
        return {"login": False}, 400

    resp = jsonify({"login": True})
    access_token = create_access_token(identity=username)
    refresh_token = create_refresh_token(identity=username)

    set_access_cookies(resp, access_token)
    set_refresh_cookies(resp, refresh_token)
    return resp, 200


@app.route("/token/refresh", methods=["POST"])
@jwt_required(refresh=True)
def refresh(endpoint):
    current_user = get_jwt_identity()
    access_token = create_access_token(identity=current_user)

    resp = redirect(url_for(endpoint))
    set_access_cookies(resp, access_token)
    return resp, 200


@app.route("/token/remove", methods=["POST"])
def logout():
    resp = jsonify({"logout": True})
    unset_jwt_cookies(resp)

    if request.cookies.get("device_id"):
        resp.set_cookie("device_id", "", expires=0)
    return resp, 200


@app.route("/api/signup", methods=["POST"])
def api_signup():
    fullname = request.json.get("fullname")
    username = request.json.get("username")
    password = request.json.get("password")

    if not s3_conn.add_user(fullname, username, password):
        return {"signup": False}, 400
    return {"signup": True}, 200


@app.route("/api/add_task", methods=["POST"])
@jwt_required()
def api_add_task():
    device_id = request.cookies.get("device_id")

    if not device_id:
        return {"message": "No device selected"}, 400

    name = request.json.get("name")
    description = request.json.get("description")
    completion = request.json.get("completion")
    priority = request.json.get("priority")
    duedate = request.json.get("duedate")

    if not s3_conn.add_task(name, description, completion, priority, duedate, device_id):
        return {"add_task": False}, 400
    return {"add_task": True}, 200


@app.route("/api/update_task", methods=["POST"])
@jwt_required()
def api_update_task():
    device_id = request.cookies.get("device_id")

    if not device_id:
        return {"message": "No device selected"}, 400

    id = request.json.get("id")
    completion = request.json.get("completion")

    if not s3_conn.update_task(id, completion, device_id):
        return {"update_task": False}, 400
    return {"update_task": True}, 200


@app.route("/api/today_tasks", methods=["POST"])
@jwt_required()
def api_today_tasks():
    device_id = request.cookies.get("device_id")

    if not device_id:
        return {"message": "No device selected"}, 400

    start = request.json.get("start")
    end = request.json.get("end")

    tasks = s3_conn.get_tasks(start, end, device_id)
    return tasks, 200


@app.route("/api/get_all_tasks")
@jwt_required()
def api_get_all_tasks():
    device_id = request.cookies.get("device_id")

    if not device_id:
        return {"message": "No device selected"}, 400

    tasks = s3_conn.get_all_tasks(device_id)
    return tasks, 200


@app.route("/api/add_event", methods=["POST"])
@jwt_required()
def api_add_event():
    device_id = request.cookies.get("device_id")

    if not device_id:
        return {"message": "No device selected"}, 400

    name = request.json.get("name")
    description = request.json.get("description")
    starttime = request.json.get("starttime")
    duration = request.json.get("duration")
    deleted = request.json.get("deleted")

    if not s3_conn.add_event(name, description, starttime, duration, deleted, device_id):
        return {"add_event": False}, 400
    return {"add_event": True}, 200


@app.route("/api/delete_event", methods=["POST"])
@jwt_required()
def api_delete_event():
    device_id = request.cookies.get("device_id")

    if not device_id:
        return {"message": "No device selected"}, 400

    id = request.json.get("id")

    if not s3_conn.delete_event(id, device_id):
        return {"delete_event": False}, 400
    return {"delete_event": True}, 200


@app.route("/api/today_events", methods=["POST"])
@jwt_required()
def api_today_events():
    device_id = request.cookies.get("device_id")

    if not device_id:
        return {"message": "No device selected"}, 400

    start = request.json.get("start")
    end = request.json.get("end")

    events = s3_conn.get_events(start, end, device_id)
    return events, 200


@app.route("/api/add_habit", methods=["POST"])
@jwt_required()
def api_add_habit():
    device_id = request.cookies.get("device_id")

    if not device_id:
        return {"message": "No device selected"}, 400

    name = request.json.get("name")
    goal = request.json.get("goal")

    if not s3_conn.add_habit(name, goal, device_id):
        return {"add_habit": False}, 400
    return {"add_habit": True}, 200


@app.route("/api/update_habit", methods=["POST"])
@jwt_required()
def api_update_habit():
    device_id = request.cookies.get("device_id")

    if not device_id:
        return {"message": "No device selected"}, 400

    id = request.json.get("id")
    date = request.json.get("date")
    completed = request.json.get("completed")
    start = request.json.get("start")
    end = request.json.get("end")

    if not s3_conn.update_habit(id, date, completed, start, end, device_id):
        return {"update_habit": False}, 400
    return {"update_habit": True}, 200


@app.route("/api/delete_habit", methods=["POST"])
@jwt_required()
def api_delete_habit():
    device_id = request.cookies.get("device_id")

    if not device_id:
        return {"message": "No device selected"}, 400

    id = request.json.get("id")

    if not s3_conn.delete_habit(id, device_id):
        return {"delete_habit": False}, 400
    return {"delete_habit": True}, 200


@app.route("/api/today_habits", methods=["POST"])
@jwt_required()
def api_today_habits():
    device_id = request.cookies.get("device_id")

    if not device_id:
        return {"message": "No device selected"}, 400

    today = request.json.get("today")
    start = request.json.get("start")

    habits = s3_conn.get_habits(today, start, device_id)
    return habits, 200


@app.route("/api/add_device", methods=["POST"])
@jwt_required()
def api_add_device():
    id = request.json.get("id")
    name = request.json.get("name")

    outcome = s3_conn.add_device(get_jwt_identity(), id, name)

    if not outcome[0]:
        return {"message": outcome[1]}, 400
    return {"message": outcome[1]}, 200


@app.route("/api/update_device", methods=["POST"])
@jwt_required()
def api_update_device():
    id = request.json.get("id")
    name = request.json.get("name")

    outcome = s3_conn.update_device(get_jwt_identity(), id, name)

    if not outcome[0]:
        return {"message": outcome[1]}, 400
    return {"message": outcome[1]}, 200


@app.route("/api/delete_device", methods=["POST"])
@jwt_required()
def api_delete_device():
    id = request.json.get("id")

    if not s3_conn.delete_device(get_jwt_identity(), id):
        return {"delete_device": False}, 400
    return {"delete_device": True}, 200


@app.route("/api/select_device", methods=["POST"])
@jwt_required()
def api_select_device():
    id = request.json.get("id")

    encrypted_id = s3_conn.encrypt_id(get_jwt_identity(), id)

    resp = jsonify({"select_device": True})
    resp.set_cookie("device_id", encrypted_id, secure=True, httponly=True)
    return resp, 200


@app.route("/api/active_device")
@jwt_required()
def api_active_device():
    id = request.cookies.get("device_id")

    if not id:
        return {"id": False}, 200

    device = s3_conn.active_device(get_jwt_identity(), id)

    if not device:
        return {"id": False}, 400
    return device, 200


@app.route("/api/get_devices")
@jwt_required()
def api_get_devices():
    devices = s3_conn.get_user_devices(get_jwt_identity())
    return devices, 200


@app.route("/api/get_users")
def api_get_users():
    users = s3_conn.get_users(data_only=True)
    return users, 200


@app.route("/login")
def login():
    return render_template("login.html")


@app.route("/signup")
def signup():
    return render_template("signup.html")


@app.route("/")
@app.route("/index")
@jwt_required()
def index():
    return render_template("index.html")


@app.route("/devices")
@jwt_required()
def devices():
    return render_template("devices.html")


@app.route("/add_task")
@jwt_required()
def add_task():
    return render_template("add-task.html")


@app.route("/admin")
def admin():
    return render_template("admin.html")


@app.route("/calendar")
@jwt_required()
def calendar():
    return render_template("calendar.html")


@app.route("/events")
@jwt_required()
def events():
    return render_template("events.html")


@app.route("/habits")
@jwt_required()
def habits():
    return render_template("habits.html")


@app.before_request
def before_request():
    if not request.cookies.get("device_id") \
            and request.endpoint in ["index", "add_task", "calendar", "events", "habits"]:
        return redirect(url_for("devices"))


@jwt.expired_token_loader
def expired_jwt_token(jwt_header, jwt_payload):
    endpoint = request.path
    try:
        # TODO: fix this
        resp, code = refresh(endpoint)
        return resp, code
    except:
        resp = redirect(url_for("login"))
        unset_jwt_cookies(resp)
        if request.cookies.get("device_id"):
            resp.set_cookie("device_id", "", expires=0)
        return resp, 302


@jwt.unauthorized_loader
def no_jwt_token(_err):
    return redirect(url_for("login"))


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8081)

from datetime import datetime, timedelta, timezone
from flask import Flask, request, render_template, redirect, url_for, jsonify
from flask_jwt_extended import (
    JWTManager, create_access_token, create_refresh_token, unset_jwt_cookies,
    set_access_cookies, set_refresh_cookies, get_jwt, get_jwt_identity, jwt_required
)
from aws_helper import AwsS3

app = Flask(__name__)
s3_conn = AwsS3()

# TODO: add proper config
app.config["JWT_SECRET_KEY"] = "TODO"
app.config["JWT_TOKEN_LOCATION"] = ["cookies"]
# app.config['JWT_COOKIE_SECURE'] = True
# app.config['JWT_COOKIE_CSRF_PROTECT'] = True
app.config["JWT_ACCESS_TOKEN_EXPIRES"] = timedelta(hours=1)

jwt = JWTManager(app)


@app.route("/test")
def test():
    return {"hello": "world"}


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


@app.after_request
def refresh_token(resp):
    try:
        exp_timestamp = get_jwt()["exp"]
        now = datetime.now(timezone.utc)
        target_timestamp = datetime.timestamp(now + timedelta(minutes=30))
        if target_timestamp > exp_timestamp:
            access_token = create_access_token(identity=get_jwt_identity())
            set_access_cookies(resp, access_token)
        return resp
    except (RuntimeError, KeyError):
        # Case where there is not a valid JWT. Just return the original response
        return resp


@app.route("/token/remove", methods=["POST"])
def logout():
    resp = jsonify({"logout": True})
    unset_jwt_cookies(resp)
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
def api_add_task():
    name = request.json.get("name")
    description = request.json.get("description")
    completion = request.json.get("completion")
    priority = request.json.get("priority")
    duedate = request.json.get("duedate")

    if not s3_conn.add_task(name, description, completion, priority, duedate):
        return {"add_task": False}, 400
    return {"add_task": True}, 200


@app.route("/api/today_tasks", methods=["POST"])
def api_today_tasks():
    start = request.json.get("start")
    end = request.json.get("end")

    tasks = s3_conn.get_tasks(start, end)
    return tasks, 200


@app.route("/api/add_event", methods=["POST"])
def api_add_event():
    name = request.json.get("name")
    starttime = request.json.get("starttime")
    duration = request.json.get("duration")

    if not s3_conn.add_event(name, starttime, duration):
        return {"add_event": False}, 400
    return {"add_event": True}, 200


@app.route("/api/delete_event", methods=["POST"])
def api_delete_event():
    id = request.json.get("id")

    if not s3_conn.delete_event(id):
        return {"delete_event": False}, 400
    return {"delete_event": True}, 200


@app.route("/api/today_events", methods=["POST"])
def api_today_events():
    start = request.json.get("start")
    end = request.json.get("end")

    events = s3_conn.get_events(start, end)
    return events, 200


@app.route("/api/today_habits", methods=["POST"])
def api_today_habits():
    habits = s3_conn.get_habits()
    return habits, 200


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


@jwt.unauthorized_loader
def no_jwt_token(_err):
    return redirect(url_for("login"))


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8081)

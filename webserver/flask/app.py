from flask import Flask, request, render_template, redirect, url_for, jsonify
from flask_jwt_extended import (
    JWTManager, create_access_token, create_refresh_token, unset_jwt_cookies,
    set_access_cookies, set_refresh_cookies, get_jwt_identity, jwt_required
)
from aws_helper import AwsS3

app = Flask(__name__)
s3_conn = AwsS3()

# TODO: add proper config
app.config["JWT_SECRET_KEY"] = "TODO"
app.config["JWT_TOKEN_LOCATION"] = ["cookies"]
# app.config['JWT_COOKIE_SECURE'] = True
# app.config['JWT_COOKIE_CSRF_PROTECT'] = True
# app.config['JWT_ACCESS_COOKIE_PATH'] = '/api/'
app.config["JWT_REFRESH_COOKIE_PATH"] = "/token/refresh"

jwt = JWTManager(app)


@app.route("/test")
def test():
    return s3_conn.load_info("task", data_only=True), 200


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
def refresh():
    current_user = get_jwt_identity()
    access_token = create_access_token(identity=current_user)

    resp = jsonify({"refresh": True})
    set_access_cookies(resp, access_token)
    return resp, 200


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


@app.route("/api/get_users")
def api_get_users():
    users = s3_conn.load_info("user", data_only=True)
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

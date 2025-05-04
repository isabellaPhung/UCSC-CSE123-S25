from flask import Flask, request, render_template, redirect, url_for, jsonify
from flask_jwt_extended import (
    JWTManager, create_access_token, create_refresh_token,
    set_access_cookies, set_refresh_cookies, get_jwt_identity, jwt_required
)
from aws_helper import AwsS3

app = Flask(__name__)
s3_conn = AwsS3()

# TODO: add proper config
app.config["JWT_SECRET_KEY"] = "TODO"
app.config['JWT_TOKEN_LOCATION'] = ['cookies']
# app.config['JWT_COOKIE_SECURE'] = True
# app.config['JWT_COOKIE_CSRF_PROTECT'] = True
# app.config['JWT_ACCESS_COOKIE_PATH'] = '/api/'
# app.config['JWT_REFRESH_COOKIE_PATH'] = '/token/refresh'

jwt = JWTManager(app)


@app.route("/api/login", methods=["POST"])
def login_authenticate():
    username = request.json.get("username")
    password = request.json.get("password")

    resp = jsonify({"login": True})
    access_token = create_access_token(identity=username)
    refresh_token = create_refresh_token(identity=username)

    set_access_cookies(resp, access_token)
    set_refresh_cookies(resp, refresh_token)
    return resp, 200


@app.route("/test")
@jwt_required()
def test():
    username = get_jwt_identity()
    return jsonify({"hello": username}), 200


@app.route("/login")
def login():
    return render_template("login.html")


@app.route("/signup")
def signup():
    return render_template("signup.html")


@app.route("/")
@app.route("/index")
def index():
    return render_template("index.html")


@app.route("/add_task")
def add_task():
    return render_template("add-task.html")


@app.route("/admin")
def admin():
    return render_template("admin.html")


@app.route("/calendar")
def calendar():
    return render_template("calendar.html")


@app.route("/events")
def events():
    return render_template("events.html")


@app.route("/habits")
def habits():
    return render_template("habits.html")


@jwt.unauthorized_loader
def no_jwt_token(_err):
    return redirect(url_for("login"))


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8081)

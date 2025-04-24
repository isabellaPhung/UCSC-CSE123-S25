from flask import Flask, request, render_template
from flask_jwt_extended import JWTManager, create_access_token, get_jwt_identity
from aws_helper import AwsS3

app = Flask(__name__)
s3_conn = AwsS3()

# TODO: add proper secret key config
app.config["JWT_SECRET_KEY"] = "TODO"
jwt = JWTManager(app)


@app.route("/api/login")
def login():
    username = request.json.get("username")
    password = request.json.get("password")

    access_token = create_access_token(identity=username)
    return {"access_token": access_token}, 200


@app.route("/")
def home():
    return render_template("index.html")


@app.route("/habits")
def habits():
    # TODO: modify home.html to properly link to habits.html
    # TODO: have different HTTP request methods?
    return render_template("habits.html")


@app.route("/tasks")
def test_tasks():
    # NOTE: this is a test endpoint
    return render_template("test_tasks.html", db_data=s3_conn.get_tasks())


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8081)

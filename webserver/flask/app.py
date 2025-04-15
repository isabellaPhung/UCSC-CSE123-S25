from flask import Flask, request, render_template
# from aws_helper import AwsS3

app = Flask(__name__)
# s3_conn = AwsS3()


@app.route("/")
def home():
    return render_template("home.html")


@app.route("/habits")
def habits():
    # TODO: modify home.html to properly link to habits.html
    # TODO: have different HTTP request methods?
    return render_template("habits.html")


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8081)

from flask import Flask, request, render_template

app = Flask(__name__)


@app.route("/")
def home():
    return render_template("home.html")


@app.route("/habits", methods=["GET"])
def habits():
    # TODO: modify home.html to properly link to habits.html
    return render_template("habits.html")


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8081)

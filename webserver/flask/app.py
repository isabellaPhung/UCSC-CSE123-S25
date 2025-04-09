from flask import Flask, request, render_template

app = Flask(__name__)


@app.route("/")
def home():
    return render_template("home.html")


@app.route("/view_schedule", methods=["GET"])
def get_schedule():
    """
    Retrieve tasks from the cloud database for the user <uuid>
    HTTP request should include in JSON: {"user": <uuid>"}
    """
    # Flask will handle cases of wrong content type/bad request
    uuid = request.json.get("user")
    # TODO: encrypt the uuid? can decrypt before querying db and encrypt before response
    return {"message": uuid}, 200


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8081)


# TEST: curl --header "Content-Type: application/json" --request GET --data '{"user": "uuid123"}' http://127.0.0.1:5000/view_schedule

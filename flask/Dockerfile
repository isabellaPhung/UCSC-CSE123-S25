FROM python:3.10.16-slim

WORKDIR /usr/src/app

COPY requirements.txt .

RUN pip3 install --no-cache-dir -r requirements.txt

COPY . .

EXPOSE 8081

CMD ["gunicorn", "--bind", "0.0.0.0:8081", "app:app"]

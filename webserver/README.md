## Usage

## Local
```bash
docker-compose up --build
```

## EC2
```bash
cd flask
docker build -t webserver .
docker run -d -p 127.0.0.1:8081:8081 --env-file .env webserver
```
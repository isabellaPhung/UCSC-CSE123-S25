FROM python:3.10.16-slim

WORKDIR /usr/src/app

RUN pip3 install Flask

COPY . .

EXPOSE 8081

CMD ["flask", "run", "--host", "0.0.0.0", "--port", "8081"]


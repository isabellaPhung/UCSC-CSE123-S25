FROM nginx:1.19-alpine

# TODO: change to certbot
RUN apk add --no-cache openssl

RUN rm /etc/nginx/nginx.conf && rm /etc/nginx/conf.d/default.conf
COPY nginx.conf /etc/nginx/
COPY app.conf /etc/nginx/conf.d/

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT /entrypoint.sh
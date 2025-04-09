FROM nginx:1.19-alpine

RUN rm /etc/nginx/nginx.conf && rm /etc/nginx/conf.d/default.conf

COPY nginx.conf /etc/nginx/

COPY app.conf /etc/nginx/conf.d/

CMD ["nginx", "-g", "daemon off;"]
FROM caddy:2.7-alpine

COPY Caddyfile /etc/caddy/Caddyfile

# Create a directory for Caddy logs
RUN mkdir -p /var/log/caddy

# Expose ports
EXPOSE 80
EXPOSE 443

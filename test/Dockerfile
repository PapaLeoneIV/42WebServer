FROM debian:buster

# install prerequisites
RUN apt-get update && \
  apt-get install -y \
  nginx \
  openssl -y


COPY nginx.conf /etc/nginx/nginx.conf

COPY entrypoint.sh /usr/local/bin/

RUN chmod +x /usr/local/bin/entrypoint.sh

ENTRYPOINT [ "entrypoint.sh" ]
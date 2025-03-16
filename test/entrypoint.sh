#!/bin/bash

if [[ -f /etc/nginx/nginx.conf ]]; then
    echo "Nginx configuration file exists at /etc/nginx/nginx.conf"
else
    echo "Error: Nginx configuration file not found!"
fi

echo "Starting Nginx..."


nginx -g "daemon off;"


if [[ $? -ne 0 ]]; then
    print_error "Error: Nginx failed to start!"
    exit 1
else
    print_success "Nginx started successfully."
fi
* dodać /files do overlaya
* /etc/nginx/nginx.conf:
```
user  www-data;
worker_processes  2;

events {
    worker_connections  1024;
}


http {
    include       mime.types;
    default_type  application/octet-stream;

    sendfile        on;

    keepalive_timeout  65;

    include /etc/nginx/sites-enabled/*; 
}
```
* uwsgi odpalić z katalogu /var/www/filehost
* migracje pytona - brakuje jednej migracji na userów
* collectstatic i STATIC_ROOT

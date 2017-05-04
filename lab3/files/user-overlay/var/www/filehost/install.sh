#!/bin/sh

[ -e install.sh ] || { echo >&2 "Please cd into the script directory before executing."; exit 1; }

python manage.py makemigrations || exit $?
python manage.py migrate || exit $?
python manage.py collectstatic || exit $?
mv "S49filehost" /etc/init.d || exit $?
/etc/init.d/S49filehost start || exit $?

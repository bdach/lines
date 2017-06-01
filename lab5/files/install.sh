#/bin/sh

# gotta figure out how to update feeds here if necessary

echo "installing prerequisites"

opkg install python3 python3-pip mpc mpd-mini || exit $?
pip3 install --upgrade -r requirements.txt || exit $?

# copy files where they need to be

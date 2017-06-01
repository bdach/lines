#/bin/sh

echo "installing prerequisites"

opkg update
opkg install python3 python3-pip mpc mpd-mini || exit $?
pip3 install --upgrade -r requirements.txt || exit $?

echo "setting up mpd directories"

mkdir -p "/var/lib/mpd" || exit $?
touch "/var/lib/mpd/tag_cache" || exit $?

echo "installing app"

mkdir /music
cp overlay/etc/mpd.conf /etc/mpd.conf || exit $?
cp -R overlay/var/www/jukebox / || exit $?

mpd

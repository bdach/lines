#!/bin/sh

# Check whether pwd is okay
[ -e make_admin.sh ] || { echo >&2 "Please cd into the script directory before executing."; exit 1; }

# Check input arguments
if [ $# -ne 1 ]
then
	echo "Usage: $0 BR2_ROOT_PATH"
	exit 1
fi

BR2_ROOT_PATH=$1

if [ -e ${BR2_ROOT_PATH}"/.config" ]
then
	echo "backing up old Buildroot configuration to .config.old"
	cp ${BR2_ROOT_PATH}"/.config" ${BR2_ROOT_PATH}"/.config.old" || exit $?
fi

echo "applying default Raspberry Pi configuration"
make -C ${BR2_ROOT_PATH} raspberrypi_defconfig || exit $?

cp admin.patch ${BR2_ROOT_PATH} || exit $?
(cd ${BR2_ROOT_PATH} && patch -N .config admin.patch)

echo "compiling image"
make -C ${BR2_ROOT_PATH}

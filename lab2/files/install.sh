#!/bin/sh

# Check whether pwd is okay
[ -e install.sh ] || { echo >&2 "Please cd into the script directory before executing."; exit 1; }

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

echo "copying simon sources"
cp -R simon ${BR2_ROOT_PATH}/../ || exit $?

echo "copying simon package files"
cp -R package ${BR2_ROOT_PATH} || exit $?

cp Config.in.patch ${BR2_ROOT_PATH}/package || exit $?
(cd ${BR2_ROOT_PATH}"/package" && patch -N Config.in Config.in.patch)

cp .config.patch ${BR2_ROOT_PATH} || exit $?
(cd ${BR2_ROOT_PATH} && patch -N .config .config.patch)

echo "compiling image"
make -C ${BR2_ROOT_PATH}

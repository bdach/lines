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

cp config.patch ${BR2_ROOT_PATH} || exit $?
(cd ${BR2_ROOT_PATH} && patch -N .config config.patch)

echo "copying overlay directory"
cp -R overlay ${BR2_ROOT_PATH}/../ || exit $?

echo "compiling image"
make -C ${BR2_ROOT_PATH}

echo "patching device tree"
#"${BR2_ROOT_PATH}/build/host-dtc-1.4.1/dtc" -I dtb -O dts -o "bcm2708-rpi-b.dts" bcm2708-rpi-b.dts
cp dts.patch ${BR2_ROOT_PATH}/output/images
(cd ${BR2_ROOT_PATH}/output/images &&
	"../build/host-dtc-1.4.1/dtc" -I dtb -O dts -o "bcm2708-rpi-b.dts" bcm2708-rpi-b.dtb &&
	patch -N bcm2708-rpi-b.dts dts.patch;
	"../build/host-dtc-1.4.1/dtc" -I dts -O dtb -o "bcm2708-rpi-b.dtb" bcm2708-rpi-b.dts) || exit $?

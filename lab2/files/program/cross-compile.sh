#!/bin/sh
BRPATH="/malina/dachb/buildroot-2016.11.2"
(
	export PATH=$BRPATH/output/host/usr/bin:$PATH
	make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- $1
)

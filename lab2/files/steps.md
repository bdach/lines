# Buildroot config steps

1. `make raspberrypi_defconfig`
2. normal menuconfig things: from tutorial (tty, mirrors, ABI, toolchain, filesystem & zipping)
3. gdb:
	- Target packages -> Debugging... -> gdb -> gdbserver
	- Toolchain -> Build cross gdb for the host
	- Build options -> build packages with debugging symbols
4. sysfs for gpio: apparently for free? idk
5. custom package

#include <fcntl.h>
#include <time.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define ERR(source) perror(source),\
	cleanup(leds, LED_COUNT),\
	cleanup(switches, SWITCH_COUNT),\
	fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
	exit(EXIT_FAILURE)

#define BLUE_LED 17
#define WHITE_LED 18
#define GREEN_LED 23
#define RED_LED 24

#define BLUE_SWITCH 10
#define WHITE_SWITCH 22
#define GREEN_SWITCH 27

#define MAX_SEQ_LEN 5
#define LED_COUNT 4
#define SWITCH_COUNT 3
#define BUF_SIZE 50

#define write_state(device, state) if (write(device.fd, state, sizeof(char)) < 0) ERR("setting state failed");
#define read_from_start(device, buf, size) do {\
	if (lseek(device.fd, 0, SEEK_SET) < 0) ERR("cannot seek to start of file");\
	if (read(device.fd, buf, size) < 0) ERR("cannot read value file");\
} while (0)
#define single_write(file, contents) do {\
	int fd;\
	if ((fd = open(file, O_WRONLY)) < 0) ERR(file);\
	if (write(fd, contents, strlen(contents)) < 0) ERR(file);\
	if (close(fd) < 0) ERR(file);\
} while (0)

typedef struct gpio_device {
	int pin;
	int fd;
} gpio_device;

gpio_device leds[LED_COUNT];
gpio_device switches[LED_COUNT];

void generate_sequence(char *buf, unsigned length);
void display_sequence(char *seq, unsigned length, gpio_device *leds);
void read_sequence(char *seq, unsigned length, gpio_device *switches, gpio_device *leds);
int read_button(gpio_device *switches);
void setup_leds(gpio_device *leds, int *pins);
void setup_switches(gpio_device *switches, int *pins);
void cleanup(gpio_device *devices, unsigned count);
void lost_game(gpio_device *switches, gpio_device *leds);

int main(void)
{
	char sequence[MAX_SEQ_LEN];
	int ledpins[LED_COUNT] = {BLUE_LED, WHITE_LED, GREEN_LED, RED_LED};
	int switchpins[SWITCH_COUNT] = {BLUE_SWITCH, WHITE_SWITCH, GREEN_SWITCH};

	setup_leds(leds, ledpins);
	setup_switches(switches, switchpins);
	generate_sequence(sequence, MAX_SEQ_LEN);
	for (unsigned i = 1; i <= MAX_SEQ_LEN; ++i) {
		display_sequence(sequence, i, leds);
		read_sequence(sequence, i, switches, leds);
	}
	cleanup(leds, LED_COUNT);
	cleanup(switches, SWITCH_COUNT);

	return 0;
}

void generate_sequence(char *buf, unsigned length)
{
	srand(time(NULL));
	for (unsigned i = 0; i < length; ++i)
		buf[i] = rand() % (LED_COUNT - 1);
}

void display_sequence(char *seq, unsigned length, gpio_device *leds)
{
	int interval = (MAX_SEQ_LEN + 1 - length) * 200000;
	usleep(1000000 - interval);
	for (unsigned i = 0; i < length; ++i) {
		usleep(interval);
		int idx = seq[i];
		write_state(leds[idx], "1");
		usleep(interval);
		write_state(leds[idx], "0");
	}
}

void setup_leds(gpio_device *leds, int *pins)
{
	const char *export_file = "/sys/class/gpio/export";

	int exportfd;
	if ((exportfd = open(export_file, O_WRONLY)) < 0)
		ERR("opening /sys/class/gpio/export failed");

	char buf[BUF_SIZE];
	for (unsigned i = 0; i < LED_COUNT; ++i) {
		snprintf(buf, BUF_SIZE, "%d", pins[i]);
		if (write(exportfd, buf, strlen(buf)) < 0)
			ERR("writing to /sys/class/gpio/export failed");
	}

	if (close(exportfd) < 0)
		ERR("closing /sys/class/gpio/export failed");

	for (unsigned i = 0; i < LED_COUNT; ++i) {
		snprintf(buf, BUF_SIZE, "/sys/class/gpio/gpio%d/direction", pins[i]);
		single_write(buf, "out");
	}

	for (unsigned i = 0; i < LED_COUNT; ++i) {
		leds[i].pin = pins[i];
		snprintf(buf, BUF_SIZE, "/sys/class/gpio/gpio%d/value", pins[i]);
		if ((leds[i].fd = open(buf, O_RDWR)) < 0) {
			ERR("opening value file failed");
		}
	}
}

void cleanup(gpio_device *devices, unsigned count)
{
	const char *unexport_file = "/sys/class/gpio/unexport";

	int unexportfd;
	if ((unexportfd = open(unexport_file, O_WRONLY)) < 0)
		ERR("opening /sys/class/gpio/export failed");

	char buf[BUF_SIZE];

	for (unsigned i = 0; i < count; ++i) {
		if (close(devices[i].fd) < 0)
			ERR("closing value file failed");
	}

	for (unsigned i = 0; i < count; ++i) {
		memset(buf, 0, BUF_SIZE);
		snprintf(buf, BUF_SIZE, "%d", devices[i].pin);
		if (write(unexportfd, buf, strlen(buf)) < 0)
			ERR("writing to /sys/class/gpio/unexport failed");
	}

	if (close(unexportfd) < 0)
		ERR("closing /sys/class/gpio/unexport failed");
}

void setup_switches(gpio_device *switches, int *pins)
{
	const char *export_file = "/sys/class/gpio/export";

	int exportfd;
	if ((exportfd = open(export_file, O_WRONLY)) < 0)
		ERR("opening /sys/class/gpio/export failed");

	char buf[BUF_SIZE];
	for (unsigned i = 0; i < SWITCH_COUNT; ++i) {
		memset(buf, 0, BUF_SIZE);
		snprintf(buf, BUF_SIZE, "%d", pins[i]);
		if (write(exportfd, buf, strlen(buf)) < 0)
			ERR("writing to /sys/class/gpio/export failed");
	}

	if (close(exportfd) < 0)
		ERR("closing /sys/class/gpio/export failed");

	for (unsigned i = 0; i < SWITCH_COUNT; ++i) {
		snprintf(buf, BUF_SIZE, "/sys/class/gpio/gpio%d/direction", pins[i]);
		single_write(buf, "in");
	}

	for (unsigned i = 0; i < SWITCH_COUNT; ++i) {
		snprintf(buf, BUF_SIZE, "/sys/class/gpio/gpio%d/edge", pins[i]);
		single_write(buf, "both");
	}

	for (unsigned i = 0; i < SWITCH_COUNT; ++i) {
		switches[i].pin = pins[i];
		snprintf(buf, BUF_SIZE, "/sys/class/gpio/gpio%d/value", pins[i]);
		if ((switches[i].fd = open(buf, O_RDWR)) < 0)
			ERR("opening value file failed");
	}
}

void read_sequence(char *seq, unsigned length, gpio_device *switches, gpio_device *leds) 
{
	for (unsigned i = 0; i < length; i++) {
		int pressed;
		// wait for a press
		while ((pressed = read_button(switches)) < 0) ;
		write_state(leds[pressed], "1");
		// wait for release
		while (read_button(switches) > 0) ;
		write_state(leds[pressed], "0");
		if (pressed != seq[i])
			lost_game(switches, leds);
	}
}

int read_button(gpio_device *switches)
{
	char val;
	// dummy read and seek before polling
	for (unsigned i = 0; i < SWITCH_COUNT; ++i)
		read_from_start(switches[i], &val, sizeof(val));

	struct pollfd fds[SWITCH_COUNT];
	for (int i = 0; i < SWITCH_COUNT; ++i)
	{
		fds[i].fd = switches[i].fd;
		fds[i].events = POLLPRI | POLLERR;
	}

	if (poll(fds, SWITCH_COUNT, -1) < 0)
		ERR("poll failed");

	for (unsigned i = 0; i < SWITCH_COUNT; ++i)
		read_from_start(switches[i], &val, sizeof(val));

	switch (poll(fds, SWITCH_COUNT, 50)) {
		case -1:
			ERR("second poll failed");
		case 0:
			// no change since last poll
			break;
		default:
			// something changed, so ignore
			return -1;
	}

	int pressed = -1;
	for (unsigned i = 0; i < SWITCH_COUNT; ++i) {
		read_from_start(switches[i], &val, sizeof(val));
		if (val == '0')
			pressed = pressed < 0 ? i : -1;
	}
	return pressed;
}

void lost_game(gpio_device *switches, gpio_device *leds)
{
	gpio_device fail_led = leds[3];
	for (unsigned i = 0; i < 3; ++i) {
		sleep(1);
		write_state(fail_led, "1");
		sleep(1);
		write_state(fail_led, "0");
	}
	cleanup(leds, LED_COUNT);
	cleanup(switches, SWITCH_COUNT);
	exit(EXIT_FAILURE);
}

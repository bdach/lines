#include <fcntl.h>
#include <time.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define ERR(source) perror(source),\
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

typedef struct gpio_device {
	int pin;
	int fd;
} gpio_device;

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
	gpio_device leds[LED_COUNT];
	gpio_device switches[LED_COUNT];

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
	for (unsigned i = 0; i < length; ++i) {
		sleep(1);
		int idx = seq[i];
		if (write(leds[idx].fd, "1", sizeof(char)) < 0)
			ERR("turning led on failed");
		sleep(1);
		if (write(leds[idx].fd, "0", sizeof(char)) < 0)
			ERR("turning led off failed");
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
		int directionfd;
		char *out = "out";
		if ((directionfd = open(buf, O_WRONLY)) < 0)
			ERR("opening direction file failed");
		if (write(directionfd, out, strlen(out)) < 0)
			ERR("writing direction failed");
		if (close(directionfd) < 0)
			ERR("closing direction file failed");
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
		int directionfd;
		char *in = "in";
		if ((directionfd = open(buf, O_WRONLY)) < 0)
			ERR("opening direction file failed");
		if (write(directionfd, in, strlen(in)) < 0)
			ERR("writing direction failed");
		if (close(directionfd) < 0)
			ERR("closing direction file failed");
	}

	for (unsigned i = 0; i < SWITCH_COUNT; ++i) {
		snprintf(buf, BUF_SIZE, "/sys/class/gpio/gpio%d/edge", pins[i]);
		int directionfd;
		char *edge = "both";
		if ((directionfd = open(buf, O_WRONLY)) < 0)
			ERR("opening edge file failed");
		if (write(directionfd, edge, strlen(edge)) < 0)
			ERR("writing edge type failed");
		if (close(directionfd) < 0)
			ERR("closing edge file failed");
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
	char state;
	for (unsigned i = 0; i < length; i++) {
		int pressed;
		// wait for a press
		while ((pressed = read_button(switches)) < 0) ;
		printf("Got press %d\n", pressed);
		state = '1';
		if (write(leds[pressed].fd, &state, sizeof(state)) < 0)
			ERR("turning on led failed");
		// wait for release
		while (read_button(switches) > 0) ;
		state = '0';
		if (write(leds[pressed].fd, &state, sizeof(state)) < 0)
			ERR("turning off led failed");
		if (pressed != seq[i])
			lost_game(switches, leds);
	}
}

int read_button(gpio_device *switches)
{
	char val;
	// dummy read and seek before polling
	for (unsigned i = 0; i < SWITCH_COUNT; ++i) {
		if (lseek(switches[i].fd, 0, SEEK_SET) < 0)
			ERR("cannot seek to start of file");
		if (read(switches[i].fd, &val, sizeof(val)) < 0)
			ERR("cannot read from value file");
	}

	struct pollfd fds[SWITCH_COUNT];
	for (int i = 0; i < SWITCH_COUNT; ++i)
	{
		fds[i].fd = switches[i].fd;
		fds[i].events = POLLPRI | POLLERR;
	}

	if (poll(fds, SWITCH_COUNT, -1) < 0)
		ERR("poll failed");

	// seek to start and read value to reset
	for (unsigned i = 0; i < SWITCH_COUNT; ++i) {
		if (lseek(switches[i].fd, 0, SEEK_SET) < 0)
			ERR("cannot seek to start of file");
		if (read(switches[i].fd, &val, sizeof(val)) < 0)
			ERR("cannot read from value file");
	}

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
		if (lseek(switches[i].fd, 0, SEEK_SET) < 0)
			ERR("cannot seek to start of file");
		if (read(switches[i].fd, &val, sizeof(val)) < 0)
			ERR("cannot read from value file");
		if (val == '0')
			pressed = pressed < 0 ? i : -1;
	}
	return pressed;
}

void lost_game(gpio_device *switches, gpio_device *leds)
{
	gpio_device fail_led = leds[3];
	char status;
	for (unsigned i = 0; i < 3; ++i) {
		sleep(1);
		status = '1';
		if (write(fail_led.fd, &status, sizeof(status)) < 0)
			ERR("turning on led failed");
		sleep(1);
		status = '0';
		if (write(fail_led.fd, &status, sizeof(status)) < 0)
			ERR("turning off led failed");
	}
	cleanup(leds, LED_COUNT);
	cleanup(switches, SWITCH_COUNT);
	exit(EXIT_FAILURE);
}

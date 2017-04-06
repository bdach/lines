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

void generate_sequence(char *buf, unsigned length);
void display_sequence(char *seq, unsigned length, int *ledfds);
int read_sequence(char *seq, unsigned length, int *switchfds);
void setup_leds(int *ledfds, int *pins);
void setup_switches(int *switchfds, int *pins);
void cleanup(int *fds, int *pins, unsigned count);

int main(void)
{
	char sequence[MAX_SEQ_LEN];
	int ledfds[LED_COUNT];
	int ledpins[LED_COUNT] = {BLUE_LED, WHITE_LED, GREEN_LED, RED_LED};
	int switchfds[SWITCH_COUNT];
	int switchpins[SWITCH_COUNT] = {BLUE_SWITCH, WHITE_SWITCH, GREEN_SWITCH};

	setup_leds(ledfds, ledpins);
	setup_switches(switchfds, switchpins);
	generate_sequence(sequence, MAX_SEQ_LEN);
	display_sequence(sequence, MAX_SEQ_LEN, ledfds);
	for (unsigned i = 0; i < 5; ++i)
		while (!read_sequence(sequence, MAX_SEQ_LEN, switchfds)) ;
	cleanup(ledfds, ledpins, LED_COUNT);
	cleanup(switchfds, switchpins, SWITCH_COUNT);

	return 0;
}

void generate_sequence(char *buf, unsigned length)
{
	srand(time(NULL));
	for (unsigned i = 0; i < length; ++i)
		buf[i] = rand() % (LED_COUNT - 1);
}

void display_sequence(char *seq, unsigned length, int *ledfds)
{
	for (unsigned i = 0; i < length; ++i) {
		int idx = seq[i];
		if (write(ledfds[idx], "1", sizeof(char)) < 0)
			ERR("turning led on failed");
		sleep(1);
		if (write(ledfds[idx], "0", sizeof(char)) < 0)
			ERR("turning led off failed");
		sleep(1);
	}
}

void setup_leds(int *ledfds, int *pins)
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
		printf("Pin %d exported\n", pins[i]);
	}

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
		printf("Set direction for pin %d\n", pins[i]);
	}

	for (unsigned i = 0; i < LED_COUNT; ++i) {
		snprintf(buf, BUF_SIZE, "/sys/class/gpio/gpio%d/value", pins[i]);
		if ((ledfds[i] = open(buf, O_RDWR)) < 0) {
			ERR("opening value file failed");
		}
		printf("value file for pin %d opened\n", pins[i]);
	}
}

void cleanup(int *fds, int *pins, unsigned count)
{
	const char *unexport_file = "/sys/class/gpio/unexport";

	int unexportfd;
	if ((unexportfd = open(unexport_file, O_WRONLY)) < 0)
		ERR("opening /sys/class/gpio/export failed");

	char buf[BUF_SIZE];

	for (unsigned i = 0; i < count; ++i) {
		if (close(fds[i]) < 0)
			ERR("closing value file failed");
		printf("closed value file for pin %d\n", pins[i]);
	}

	for (unsigned i = 0; i < count; ++i) {
		snprintf(buf, BUF_SIZE, "%d", pins[i]);
		if (write(unexportfd, buf, strlen(buf)) < 0)
			ERR("writing to /sys/class/gpio/export failed");
		printf("Pin %d unexported\n", pins[i]);
	}
}

void setup_switches(int *switchfds, int *pins)
{
	const char *export_file = "/sys/class/gpio/export";

	int exportfd;
	if ((exportfd = open(export_file, O_WRONLY)) < 0)
		ERR("opening /sys/class/gpio/export failed");

	char buf[BUF_SIZE];
	for (unsigned i = 0; i < SWITCH_COUNT; ++i) {
		snprintf(buf, BUF_SIZE, "%d", pins[i]);
		if (write(exportfd, buf, strlen(buf)) < 0)
			ERR("writing to /sys/class/gpio/export failed");
		printf("Pin %d exported\n", pins[i]);
	}

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
		printf("Set direction for pin %d\n", pins[i]);
	}

	for (unsigned i = 0; i < SWITCH_COUNT; ++i) {
		snprintf(buf, BUF_SIZE, "/sys/class/gpio/gpio%d/edge", pins[i]);
		int directionfd;
		char *edge = "rising";
		if ((directionfd = open(buf, O_WRONLY)) < 0)
			ERR("opening edge file failed");
		if (write(directionfd, edge, strlen(edge)) < 0)
			ERR("writing edge type failed");
		if (close(directionfd) < 0)
			ERR("closing edge file failed");
		printf("Set edge type for pin %d\n", pins[i]);
	}

	for (unsigned i = 0; i < SWITCH_COUNT; ++i) {
		snprintf(buf, BUF_SIZE, "/sys/class/gpio/gpio%d/value", pins[i]);
		if ((switchfds[i] = open(buf, O_RDWR)) < 0) {
			ERR("opening value file failed");
		}
	}
}

int read_sequence(char *seq, unsigned length, int *switchfds)
{
	char buf[BUF_SIZE];
	for (unsigned i = 0; i < SWITCH_COUNT; ++i) {
		if (read(switchfds[i], buf, BUF_SIZE) < 0)
			ERR("cannot read from value file");
		if (lseek(switchfds[i], 0, SEEK_SET) < 0)
			ERR("cannot seek to start of file");
	}

	struct pollfd fds[SWITCH_COUNT];
	for (int i = 0; i < SWITCH_COUNT; ++i)
	{
		fds[i].fd = switchfds[i];
		fds[i].events = POLLPRI | POLLERR;
	}

	if (poll(fds, SWITCH_COUNT, -1) < 0)
		ERR("poll failed");

	for (unsigned i = 0; i < SWITCH_COUNT; ++i) {
		if (!(fds[i].revents & POLLPRI))
			continue;
		if (read(switchfds[i], buf, BUF_SIZE) < 0)
			ERR("cannot read from value file");
		if (lseek(switchfds[i], 0, SEEK_SET) < 0)
			ERR("cannot seek to start of file");
	}

	switch (poll(fds, SWITCH_COUNT, 50)) {
		case -1:
			ERR("second poll failed");
		case 0:
			break;
		default:
			return 0;
	}

	for (unsigned i = 0; i < SWITCH_COUNT; ++i) {
		if (read(switchfds[i], buf, BUF_SIZE) < 0)
			ERR("cannot read from value file");
		printf("button %d: %s", i, buf);
	}
	return 1;
}


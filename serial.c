#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <termios.h>

#define BUF_SIZE	256

static int BAUD(int baud)
{
	int br_bit;

	switch (baud) {
	case   9600: br_bit = B9600;   break;
	case  19200: br_bit = B19200;  break;
	case  38400: br_bit = B38400;  break;
	case  57600: br_bit = B57600;  break;
	case 115200: br_bit = B115200; break;
	default:	 br_bit = B115200;
	}

	return br_bit;
}

static void set_termios(int fd, int baud)
{
	struct termios option = { 0 };

	tcgetattr(fd, &option);

	cfmakeraw(&option);
	cfsetspeed(&option, BAUD(baud));
	option.c_cc[VTIME] = 0;
	option.c_cc[VMIN] = 1;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &option);
}

static void write_routine(void *arg)
{
	int fd = *(int *)arg;
	char wrbuf[BUF_SIZE];
	int n;

	while (1) {
		n = read(0, wrbuf, BUF_SIZE - 1);
		if (n > 1)
			write(fd, wrbuf, n);
		else
			write(1, ">> ", 3);

		usleep(10000);
	}
}

static void read_routine(int fd, int hex)
{
	char rdbuf[BUF_SIZE];
	char hexstr[4];
	int n, i;

	while (1) {
		n = read(fd, rdbuf, BUF_SIZE - 1);
		if (n <= 0) {
			usleep(2);
			continue;
		}

		if (hex) {
			for (i = 0; i < n; i++) {
				snprintf(hexstr, 4, " %02X", rdbuf[i]);
				write(1, hexstr, 3);
			}
		} else {
			write(1, rdbuf, n);
		}
	}
}

void usage(void)
{
	printf("\nusage:  serial [-x] [-b <baudrate>] /dev/ttyXXX\n\n"
		"\t-b baudrate\tbaudrate (default: 115200)\n"
		"\t-x	      \toutput in hex format\n"
		"\n");
}

int main(int argc, char *argv[])
{
	char *ttyname;
	pthread_t write_thread;
	int baud = 115200;
	int hex = 0;
	int opt;
	int fd;


	if (argc < 2) {
		usage();
		exit(-1);
	}

	while ((opt = getopt(argc, argv, "b:xh")) != -1) {
		switch (opt) {
		case 'b':
			sscanf(optarg, "%d", &baud);
			break;
		case 'x':
			hex = 1;
			break;
		case 'h':
			usage();
			exit(0);
			break;
		default:
			usage();
			exit(-1);
		}
	}

	ttyname = argv[argc-1];
	if (strncmp(ttyname, "/dev/tty", 7)) {
		printf("unknow tty: %s\n", ttyname);
		usage();
	}

	fd = open(ttyname, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		printf("failed to open %s\n", ttyname);
		exit(1);
	}

	set_termios(fd, baud);

	if (pthread_create(&write_thread, NULL, (void *)write_routine, &fd)) {
		printf("pthread create failed\n");
		return -1;
	}

	read_routine(fd, hex);

	return 0;
}

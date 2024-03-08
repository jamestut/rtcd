#include <linux/rtc.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <err.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "rtclib.h"

#define DEF_RTC_PATH "/dev/rtc0"

enum OperMode {
	OP_READ_RTC,
	OP_READ_SYS,
	OP_UNKNOWN
};

static void print_help();

static void read_rtc_time_i(const char * dev, struct tm * time, time_t * epoch);

static void read_sys_time_i(struct tm * time, time_t * epoch);

int main(int argc, const char ** argv) {
	enum OperMode opmode = OP_UNKNOWN;
	const char * rtcdev = DEF_RTC_PATH;
	bool set_system_time = false;
	if (argc < 2) {
		opmode = OP_READ_RTC;
	} else if (argc >= 2 && argc <= 3) {
		if (strcmp("-h", argv[1]) == 0) {
			print_help();
			return 0;
		} else if (strcmp("-s", argv[1]) == 0) {
			opmode = OP_READ_RTC;
			set_system_time = true;
		} else if (strcmp("-rs", argv[1]) == 0) {
			opmode = OP_READ_SYS;
		} else if (strcmp("-r", argv[1]) == 0) {
			opmode = OP_READ_RTC;
		}
		if (argc == 3) {
			rtcdev = argv[2];
		}
	}

	struct tm time;
	time_t epoch;

	switch (opmode) {
	case OP_READ_RTC:
		read_rtc_time_i(rtcdev, &time, &epoch);
		break;
	case OP_READ_SYS:
		read_sys_time_i(&time, &epoch);
		break;
	default:
		print_help();
		return 1;
	}

	printf("year  = %d\n", time.tm_year);
	printf("month = %d\n", time.tm_mon);
	printf("day   = %d\n", time.tm_mday);
	printf("hour  = %d\n", time.tm_hour);
	printf("min   = %d\n", time.tm_min);
	printf("sec   = %d\n", time.tm_sec);
	printf("epoch = %lu\n", epoch);

	if (set_system_time) {
		if (set_system_time_from_utc(epoch) < 0) {
			err(1, "Error setting system time");
		}
		puts("Set system time done!");
	}
}

static void read_rtc_time_i(const char * dev, struct tm * time, time_t * epoch) {
	int fd = open(dev, O_RDONLY);
	if (fd < 0) {
		err(1, "Error opening RTC device");
	}
	if (read_rtc_time(fd, time) < 0) {
		err(1, "Error reading RTC");
	}
	*epoch = timegm(time);
}

static void read_sys_time_i(struct tm * time, time_t * epoch) {
	*epoch = read_sys_time();
	gmtime_r(epoch, time);
}

void print_help() {
	puts("Usage: rtcctl (options) [RTC path]");
	puts("Available options are:");
	puts(" -h   Show this help message.");
	puts(" -s   Set system time from RTC.");
	puts(" -rs  Read system time.");
	puts(" -r   Read RTC time. (default)");
	printf("RTC path is default '%s'\n", DEF_RTC_PATH);
}

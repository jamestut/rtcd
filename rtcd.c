#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include "rtclib.h"

#define MAX_DELTA 2
#define DEF_INT 5

#define DEF_RTC_PATH "/dev/rtc0"

static void print_help();

static inline time_t ttabs(time_t v) {
	return v >= 0 ? v : -v;
}

int main(int argc, const char ** argv) {
	const char * dev = DEF_RTC_PATH;
	int interval = DEF_INT;

	// parse args
	for (int argi = 1; argi < argc; ++argi) {
		if (strcmp(argv[argi], "-i") == 0) {
			interval = atoi(argv[++argi]);
			if (interval < 0) {
				fputs("Interval must be a positive number.\n", stderr);
				return 1;
			}
			continue;
		} else if (strcmp(argv[argi], "-d") == 0) {
			dev = argv[++argi];
			continue;
		} else if (strcmp(argv[argi], "-h") == 0) {
			print_help();
			return 0;
		} else {
			print_help();
			return 1;
		}
	}

	printf("Monitoring '%s' with interval of %d sec\n", dev, interval);

	int fd = open(dev, O_RDONLY);
	if (fd < 0) {
		err(1, "Error opening RTC device");
	}

	for(;;usleep(interval * 1000 * 1000)) {
		struct tm rtc_tm;
		if (read_rtc_time(fd, &rtc_tm) < 0) {
			warn("Error reading RTC time");
			continue;
		}
		time_t rtctime = timegm(&rtc_tm);
		time_t systime = read_sys_time();
		if (ttabs(rtctime - systime) > MAX_DELTA) {
			puts("System time differ from RTC time. Synching.");
			set_system_time_from_utc(rtctime);
		}
	}
}

static void print_help() {
	puts("Usage: rtcd [OPTIONS])");
	puts("Options are:");
	printf(" -i  Set the refresh interval in second (default %d)\n", DEF_INT);
	printf(" -d  RTC device path (default %s)\n", DEF_RTC_PATH);
}

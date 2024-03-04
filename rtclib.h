#pragma once

#include <time.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>

static inline int read_rtc_time(int fd, struct tm * time) {
	return ioctl(fd, RTC_RD_TIME, time);
}

static inline time_t read_sys_time() {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_sec; // local time in seconds
}

static inline time_t utc_to_localtime_epoch(time_t epoch) {
	struct tm tm;
	gmtime_r(&epoch, &tm);
	time_t gmepoch = mktime(&tm);
	return epoch + (epoch - gmepoch);
}

static inline int set_system_time_from_utc(time_t epoch) {
	struct timespec tv = {
		.tv_sec = epoch
	};
	return clock_settime(CLOCK_REALTIME, &tv);
}

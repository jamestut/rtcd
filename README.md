# rtcd

This app periodically syncs Linux' system time from the RTC device (e.g. `/dev/rtc0`). This app will only set the system time if it drifts from the RTC's time by at least 2 seconds. Unlike GNU's `hwclock`, this app directly reads the RTC device without waiting for it to tick.

Useful for use inside virtual machine where typically Linux keeps track of its time from the CPU timer counter, which will be paused when host machine is put to standby, thus making the Linux guest's time out of sync.

## Usage

The sync period and the RTC device can be defined from `rtcd`'s command line arguments. `rtcd` will continue to run in the foreground.

The `rtcctl` program can be used to test query system's time and RTC device, and also to manually sync system's time from RTC.

### Systemd Service

For systemd-based Linux, `rtcd` can be configured to automatically start. Following are the steps to configure auto start:

- Put the `rtcd.service` to `/etc/systemd/system`.

- Enable the service by invoking `systemctl enable rtcd.service`.

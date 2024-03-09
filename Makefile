ALL := rtcctl rtcd
RTCCTL_O := rtcctl.o
RTCD_O := rtcd.o

CFLAGS := -std=gnu17 $(CFLAGS)

all: $(ALL)

rtcctl: $(RTCCTL_O) *.h
	$(CC) $(CFLAGS) -o $@ $(RTCCTL_O)

rtcd: $(RTCD_O) *.h
	$(CC) $(CFLAGS) -o $@ $(RTCD_O)

.o:
	$(CC) $(CFLAGS) $@.c $(LDFLAGS) -o $@

.PHONY: clean cleanartifacts

clean:
	rm -f $(ALL) *.o

cleanartefacts:
	rm -f *.zip

#include <stdio.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


typedef struct {
	int max;
	int min;
} Range;

typedef struct {
	Range x, y;
} Calibration;

int calc(int current, Range r) {
	int percent = (((float) current - r.min) / (r.max - r.min) * 100.0);
	if(percent < 0) {
		return 0;
	} else if(percent > 100) {
		return 100;
	}

	return percent;
}

int main() {
	Calibration c;
	/*
	c.x.min = 430;
	c.x.max = 2500;

	c.y.min = 1150;
	c.y.max = 2950;
	*/

	c.x.min = 211;
	c.x.max = 3500;

	c.y.min = 506;
	c.y.max = 3500;

	int fd = open("/dev/input/event0", O_RDONLY);
	if(fd < 0) {
		perror("open");
		return 1;
	}

	struct input_event evt;

	int X, Y;
	while(read(fd, &evt, sizeof(evt)) == sizeof(evt)) {
		if(evt.type == EV_ABS) {
			if(evt.code == ABS_X) {
				X = evt.value;
			} else if(evt.code == ABS_Y) {
				Y = evt.value;
			}
			//printf("%d %d %d\n", evt.type, evt.code, evt.value);
		} else if(evt.type == EV_SYN) {
			printf("X=%d Y=%d XP=%d XY=%d\n", X, Y, calc(X, c.x), calc(Y, c.y));
			fflush(stdout);
		}
	}

	return 0;
}

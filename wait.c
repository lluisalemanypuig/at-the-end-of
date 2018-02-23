#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

	char buf[256];
	sprintf(buf, "pidof -x %s > /dev/null", argv[1]);

	while (system(buf) == 0) {
		sleep(1.0);
	}

	return 0;
}

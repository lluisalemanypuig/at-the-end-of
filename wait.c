#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

	size_t len = strlen(argv[1]);
	if (len + 21 > 256) {
		fprintf(stderr, "Error: name is too long\n");
		return 1;
	}

	char buf[256];
	int r = sprintf(buf, "pidof -x %s > /dev/null", argv[1]);

	if (r < 0) {
		fprintf(stderr, "Error when creating buffer\n");
		return 1;
	}

	while (system(buf) == 0) {
		sleep(1.0);
	}

	return 0;
}

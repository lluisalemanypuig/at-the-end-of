#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

void show_usage() {
	printf("At-the-end-of's options:\n");
	printf("    [-h, --help]: show the help\n");
	printf("    [-p, --process-name] s: specify process to name to wait for completion\n");
	printf("    [-i, --interval] i: specify the period in seconds in which the program\n");
	printf("        will be polling the status of the process. This parameter is optional\n");
	printf("        and is set to 1.0 by default\n");
}

void parse_seconds(const char *str, double *s, char *should_exit) {
	
	char *err;
	*s = strtod(str, &err);
	
	/*
	according to the manual:
	
	If  no conversion is performed, zero is returned and the value of
	nptr is stored in the location referenced by endptr.
	*/
	if (err == str) {
		fprintf(stderr, "Error: no conversion of '%s' into double could be performed\n", str);
		*should_exit = 1;
		return;
	}
	
	/*
	according to the manual:
	
	If the correct value would cause overflow, plus or minus HUGE_VAL
	(HUGE_VALF, HUGE_VALL) is returned (according  to  the  sign  of
	the value), and ERANGE is stored in errno.
	
	If the correct value would cause underflow, zero is returned and
	ERANGE is stored in errno.
	*/
	if (errno == ERANGE && (*s == HUGE_VAL || *s == HUGE_VALF || *s == HUGE_VALL)) {
		fprintf(stderr, "Error: value '%s' causes overflow\n", str);
		*should_exit = 1;
		return;
	}
	if (errno == ERANGE && *s == 0.0) {
		fprintf(stderr, "Error: value '%s' causes underflow\n", str);
		*should_exit = 1;
		return;
	}
	
	*should_exit = 0;
}

int main(int argc, char *argv[]) {
	char *p_name = NULL;
	double s_time = 1.0;
	char error_parsing_interval = 0;

	// parse options: if help option is found then print usage and exit
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			show_usage();
			return 0;
		}
		else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--process-name") == 0) {
			p_name = argv[i + 1];
			++i;
		}
		else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interval") == 0) {
			parse_seconds(argv[i + 1], &s_time, &error_parsing_interval);
			++i;
		}
	}
	
	if (error_parsing_interval == 1) {
		fprintf(stderr, "An error occurred while parsing interval\n");
		return 1;
	}
	
	if (p_name == NULL) {
		fprintf(stderr, "Error: process name not given\n");
		fprintf(stderr, "    Use option [-h, --help] the see the usage\n");
		return 1;
	}
	
	size_t len = strlen(p_name);
	if (len > 256) {
		fprintf(stderr, "Error: name is too long\n");
		return 1;
	}
	
	fprintf(stdout, "Polling state of processes named '%s' every '%f' seconds\n", p_name, s_time);
	
	char buf[256 + 21]; // allow process name of 256 characters
	int r = sprintf(buf, "pidof -x %s > /dev/null", p_name);
	
	if (r < 0) {
		fprintf(stderr, "Error when creating buffer\n");
		return 1;
	}
	
	while (system(buf) == 0) {
		sleep(s_time);
	}
	
	return 0;
}

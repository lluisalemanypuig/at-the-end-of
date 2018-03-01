#include <sys/resource.h>
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
	printf("    [-a, --abort] t: abort after t seconds.\n");
	printf("        A negative value is interpreted as 'never' (default)\n");
}

inline double now() {
	struct timeval tim;
	struct rusage ru;
	getrusage(RUSAGE_SELF, &ru);
	tim = ru.ru_utime;
	return ((double)tim.tv_sec + ((double)tim.tv_usec)/1000000.0);
}

// return the time in seconds 
inline double elapsed_time(double begin, double end) {
	return (end - begin >= 0.0 ? end - begin : 0.0);
}

inline double min(double a, double b) {
	return (a < b ? a : b);
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
	char *p_name = NULL;	// process name
	double s_time = 1.0;	// polling interval time
	double a_time = -1.0;	// abort time
	char error_parsing_interval = 0;
	char error_parsing_abort = 0;

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
		else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--abort") == 0) {
			parse_seconds(argv[i + 1], &a_time, &error_parsing_abort);
			++i;
		}
	}

	if (error_parsing_interval == 1) {
		fprintf(stderr, "An error occurred while parsing interval time\n");
		return 1;
	}

	if (error_parsing_abort == 1) {
		fprintf(stderr, "An error occurred while parsing abort time\n");
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

	char buf[256 + 21]; // allow process name of 256 characters
	int print_res = sprintf(buf, "pidof -x %s > /dev/null", p_name);
	if (print_res < 0) {
		fprintf(stderr, "Error when creating buffer\n");
		return 1;
	}

	// do not start execution for small abort times
	if (a_time > 0 && a_time <= 0.2) {
		return 0;
	}
	
	const double begin = now();
	double end = begin;

	if (a_time < 0.0) {
		// run forever (or until the process finishes)
		while (system(buf) == 0) {
			usleep(s_time*1000000.0);
		}
		return 0;
	}

	// poll the system every 's_time' seconds until we are
	// at the last poll before the aborting time

	while (system(buf) == 0 && elapsed_time(begin, end) + s_time < a_time) {
		printf("1\n");
		usleep(s_time*1000000.0);
		end += s_time;
		printf("elapsed time: %f\n", elapsed_time(begin, end));
	}

	// poll the system a bit more frequently
	// until the abort time is exhausted
	const double r = a_time - elapsed_time(begin, end);
	while (system(buf) == 0 && elapsed_time(begin, end) < a_time) {
		printf("1\n");
		usleep((r/s_time)*1000000.0);
		end += r/s_time;
		printf("elapsed time: %f\n", elapsed_time(begin, end));
	}

	return 0;
}

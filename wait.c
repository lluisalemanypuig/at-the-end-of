/*********************************************************************
 * At the end of
 * Copyright (C) 2018  Lluís Alemany Puig
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * Contact: Lluís Alemany Puig (lluis.alemany.puig@gmail.com)
 * 
 *********************************************************************/

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
	int arg_i;
	for (arg_i = 1; arg_i < argc; ++arg_i) {
		if (strcmp(argv[arg_i], "-h") == 0 || strcmp(argv[arg_i], "--help") == 0) {
			show_usage();
			return 0;
		}
		else if (strcmp(argv[arg_i], "-p") == 0 || strcmp(argv[arg_i], "--process-name") == 0) {
			p_name = argv[arg_i + 1];
			++arg_i;
		}
		else if (strcmp(argv[arg_i], "-i") == 0 || strcmp(argv[arg_i], "--interval") == 0) {
			parse_seconds(argv[arg_i + 1], &s_time, &error_parsing_interval);
			++arg_i;
		}
		else if (strcmp(argv[arg_i], "-a") == 0 || strcmp(argv[arg_i], "--abort") == 0) {
			parse_seconds(argv[arg_i + 1], &a_time, &error_parsing_abort);
			++arg_i;
		}
	}

	// error control
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
	
	// start polling the system for process status

	// Method 1. If no abort time was specified just poll the
	// process' status every 'sleep time' seconds. Terminate
	// execution when all processes have finished.
	if (a_time < 0.0) {
		// run forever (or until the process finishes)

		while (system(buf) == 0) {
			sleep(s_time);
		}
		return 0;
	}

	// Method 2. Poll the system every 's_time' seconds until
	// we are at the last poll before the aborting time.
	double end = 0.0;
	while (system(buf) == 0 && end + s_time < a_time) {
		sleep(s_time);
		end += s_time;
	}

	// Method 2. (continued) Poll the system a bit more frequently
	// until the abort time is exhausted.

	// remaining time to wait
	const double r = a_time - end;

	// new polling time (seconds, microseconds)
	const double ns_time = r/s_time;
	const double uns_time = ns_time*1000000.0;

	while (system(buf) == 0 && end < a_time) {
		usleep(uns_time);
		end += ns_time;
	}

	return 0;
}

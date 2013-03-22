/*********************************************************************
 *  ____                      _____      _                           *
 * / ___|  ___  _ __  _   _  | ____|_ __(_) ___ ___ ___  ___  _ __   *
 * \___ \ / _ \| '_ \| | | | |  _| | '__| |/ __/ __/ __|/ _ \| '_ \  *
 *  ___) | (_) | | | | |_| | | |___| |  | | (__\__ \__ \ (_) | | | | *
 * |____/ \___/|_| |_|\__, | |_____|_|  |_|\___|___/___/\___/|_| |_| *
 *                    |___/                                          *
 *                                                                   *
 *********************************************************************
 * Copyright 2010 Sony Ericsson Mobile Communications AB.            *
 * All rights, including trade secret rights, reserved.              *
 *********************************************************************/
/**
 * @file main.c
 *
 * @author Radovan Lekanovic (radovan.lekanovic@sonyericsson.com)
 * @author Paul Ring (paul.ring@sonyericsson.com)
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <fcntl.h>
#include <time.h>
#include <signal.h>

#include "utils.h"
#include "parser.h"

clock_t start, end;
uint32 elapsed;
extern int loop_forever;

void signalHandler(int signum)
{
	switch(signum)
	{
		case SIGTERM:
		case SIGQUIT:
		case SIGINT:
			loop_forever = 0;
			break;
	}
}

static int
exitEnv()
{
	char buff[MAXLINE];
	int ret;

		// Disable the tracer
	memset(buff, 0, sizeof(buff));
	if (TARGET == 0) {
		sprintf(buff, "sudo -s %s", "\'echo nop > /sys/kernel/debug/tracing/current_tracer\'");
	} else {
		strcat(buff, "echo nop > /sys/kernel/debug/tracing/current_tracer");
	}

	if ((ret = system(buff)) < 0) {
		printf("Cannot \"echo nop > /sys/kernel/debug/tracing/current_tracer\"\n");
	}

	// Unmount the /sys/kernel/debug
	memset(buff, 0, sizeof(buff));
	if (TARGET == 0) {
		sprintf(buff, "sudo -s %s", "umount /sys/kernel/debug");
	} else {
		strcat(buff, "umount /sys/kernel/debug");
	}

	if ((ret = system(buff)) < 0) {
		printf("Warning: Cannot \"umount /sys/kernel/debug\" ret=%d\n", ret);
	}

	return ret;
}

static int
initEnv()
{
	char buff[MAXLINE];
	int ret;
	struct stat fileStat;

	// Unmount the /sys/kernel/debug
	memset(buff, 0, sizeof(buff));
	if (TARGET == 0) {
		sprintf(buff, "sudo -s %s", "umount /sys/kernel/debug");
	} else {
		strcat(buff, "umount /sys/kernel/debug");
	}

	if (stat("/sys/kernel/debug/tracing/current_tracer", &fileStat) == 0) {
		// Check if the "/sys/kernel/debug" is mounted before unmounting it
		// otherwise we will get an error
		if ((ret = system(buff)) < 0) {
			printf("Warning: Cannot \"umount /sys/kernel/debug\" ret=%d\n", ret);
		}
	}

	// Mount the /sys/kernel/debug
	memset(buff, 0, sizeof(buff));
	if (TARGET == 0) {
		sprintf(buff, "sudo -s %s", "mount -r -w -t debugfs nodev /sys/kernel/debug");
	} else {
		strcat(buff, "mount -r -w -t debugfs nodev /sys/kernel/debug");
	}
	if ((ret = system(buff)) < 0) {
		printf("Cannot \"mount -r -w -t debugfs nodev /sys/kernel/debug\"\n");
		goto exit;
	}

	// Enable the tracer
	memset(buff, 0, sizeof(buff));
	if (TARGET == 0) {
		sprintf(buff, "sudo -s %s", "\'echo sched_switch > /sys/kernel/debug/tracing/current_tracer\'");
	} else {
		strcat(buff, "echo sched_switch > /sys/kernel/debug/tracing/current_tracer");
	}

	if ((ret = system(buff)) < 0) {
		printf("Cannot \"echo sched_switch > /sys/kernel/debug/tracing/current_tracer\"\n");
		goto exit;
	}

exit:
	return ret;
}

int main(int argc, char** argv)
{
	int ret;
	int c = 0;
	char* path = NULL;

	init();

	signal(SIGQUIT, &signalHandler);
	signal(SIGTERM, &signalHandler);
	signal(SIGINT, &signalHandler);

	while ((c = getopt (argc, argv, "f:")) != -1) {
		switch(c) {
			case 'f':
				path = strdup(optarg);
				break;
		}
	}
	// Initialize debugging environment
	if ((ret = initEnv()) < 0) {
		printf("Cannot set debugging environment, exiting. (ret = %d)\n", ret);
		exit(EXIT_FAILURE);
	}

	// Do the actual processing
	// Use the hardcoded "trace_pipe" file if no path is defined
	startParse((path != NULL) ? path : "/sys/kernel/debug/tracing/trace_pipe");

	// Clean up and exit debug environment
	del();
	free(path);
	if ((ret = exitEnv()) < 0) {
		printf("Cannot unset debugging environment (ret = %d)\n", ret);
	}

	return 0;
}


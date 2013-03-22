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
 * @file parser.c
 *
 * @author Radovan Lekanovic (radovan.lekanovic@sonyericsson.com)
 * @author Paul Ring (paul.ring@sonyericsson.com)
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include "utils.h"
#include "parser.h"

int loop_forever = 1;

static int is_line_ok(char* s)
{
	if ( (strnlen(s,MAXLINE) <= 10) ) {
		printf("Line error, line too short = %s\n",s);
		return 0;
	}

	if( s[0] == '#' || s[0] == '*') {
		printf("Line error, ignoring line = %s\n",s);
		return 0;
	}

	return 1;
}

static int skipwhite(char* str)
{
	char* tmp = str;
	int i=0;

	while( isspace(tmp[i]) ) {
		if( i >= MAXLINE ) {
			printf("String corrupt\n");
			exit(EXIT_FAILURE);
		}
		i++;
	}
	return i;
}
static void processLine(char* line)
{
	char* name=NULL,*time=NULL;
	int name_start=0, name_end=0,time_start,time_end,name_len,time_len;
	double t=0.0;

	if ( !is_line_ok(line)) {
		return;
	}

	name_start = skipwhite(line);

	//name = strpbrk(line,"[");
	//name_end = ((int)(name - line) - 1);
	name_end = 23;

	//name = strpbrk(line,"]");
	//time_start = ((int)(name - line) + 1);
	time_start = 29;

	name = strpbrk( (line+time_start),":");

	time_end = (int)(name - line);

	name_len = (name_end - name_start) ;
	time_len = (time_end - time_start) ;

	name = malloc(name_len + 1);
	time = malloc(time_len + 1);

	memset(name,0,name_len+1);
	memset(time,0,time_len+1);

	memcpy(name,&line[name_start],name_len);
	memcpy(time,&line[time_start],time_len);

	t = atof(time);

	add_to_list(name,t);

	free(name);
	free(time);

}
static void looper(FILE* pfile)
{
	char* p;
	char line[MAXLINE];

	while (loop_forever) {
		p=fgets(line,MAXLINE,pfile);
		processLine(&line[0]);
	}
}

void startParse(char* path)
{
	FILE* pFile;

	if ((pFile = fopen(path, "r")) == NULL) {
		printf("Error: open file failed '%s'\n", path);
		exit(EXIT_FAILURE);
	}

	// The actual processing is done here
	looper(pFile);

	// Close the file
	fclose(pFile);

	// Save the date to the file
	save_values_to_file();

	/*
	* stdin = 0
	* stdout = 1
	* stderr = 2
	*/
}







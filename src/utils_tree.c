/**
 * @file utils_tree.c
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "tree.h"
#include "utils.h"

static double start_timestamp;
static double last_timestamp;

#define MAX_NAME_LEN	80
typedef struct node_s node_t;

struct node_s {
	RB_ENTRY(node_s) link;
	char* name;
	int called;
	double total_time;
	double time_stamp;
};


struct nodelist_s {
	struct nodelist_s* pNext;
	struct nodelist_s* pPrev;
	node_t* data;
};

static struct nodelist_s treenodes;

RB_HEAD(mytree, node_s);
static struct mytree tree;
static struct node_s last_to_run={
	.name = NULL,
	.called = 0,
	.total_time = 0.0,
	.time_stamp = 0.0,
};

void myprint(char *, node_t *);
void print_all();

static int node_cmp(node_t* e1,  node_t* e2) {
	int ret = 0;

	if(e1->name != NULL && e2->name != NULL) {
		ret = strncmp(e1->name,e2->name,MAXLINE);
	}

	return ret;
}

RB_GENERATE(mytree, node_s, link, node_cmp);

void init()
{
	RB_INIT(&tree);
}

void add_to_list(char* name, double timestamp)
{
	node_t *e, *item;

	if ((e = malloc(sizeof(node_t))) == NULL) {
		perror("malloc");
	}
	e->name = strdup(name);
	e->called = 1;
	e->time_stamp = timestamp;

	if ( last_to_run.name == NULL) { //first call
		last_to_run.name = malloc(MAX_NAME_LEN);
		strncpy(last_to_run.name,e->name,MAX_NAME_LEN);
		last_to_run.called = 1;
		last_to_run.time_stamp = timestamp;
		start_timestamp = e->time_stamp = timestamp;

		e->total_time = 0.0;
		RB_INSERT(mytree,&tree,e);
		return;
	}

	if ((item = RB_FIND(mytree,&tree,e)) == NULL){ //If not in three, add it
		item = RB_FIND(mytree,&tree,&last_to_run);
		item->total_time += (timestamp - last_to_run.time_stamp);
		e->total_time = 0.0;
		RB_INSERT(mytree, &tree, e);

		strncpy(last_to_run.name,e->name,MAX_NAME_LEN);
		last_to_run.called = e->called;
		last_to_run.total_time = e->total_time;
		last_to_run.time_stamp = e->time_stamp;

	} else { //otherwise increment called parameter
		node_t *prev;

		item->called++;
		item->time_stamp = timestamp;

		prev = RB_FIND(mytree,&tree,&last_to_run);
		prev->total_time += (timestamp - last_to_run.time_stamp);

		strncpy(last_to_run.name,item->name,MAX_NAME_LEN);
		last_to_run.called = item->called;
		last_to_run.total_time = item->total_time;
		last_to_run.time_stamp = item->time_stamp;

		free(e->name);//free name since its already in list
		free(e);
	}
	last_timestamp = timestamp;

	return;
}
void myprint(char *str, node_t *o)
{
	printf("[%s]\nname = %s\ncalled = %d\ntime_stamp = %f\ntotal_time = %f\n\n",
		str,o->name,
		o->called,
		o->time_stamp,
		o->total_time);

}
void print_all()
{
	char buffer[MAXLINE];
	node_t *node;
	double tmp=0.0;

	RB_FOREACH(node, mytree, &tree) {
		sprintf(buffer,"%d %s %f %f\n",(int)node->called,node->name,node->total_time,node->time_stamp);
		tmp += node->total_time;
		printf("%s\n",buffer);
		memset(buffer,0,MAXLINE);
	}
}

static void
addTreeItem(node_t* node) {
	struct nodelist_s* temp;
	struct nodelist_s* foundNode = NULL;

	temp = &treenodes;
	do {
		if (temp->data != NULL) {
			double timenode = node->total_time;
			double timeitem = temp->data->total_time;
			if (timenode <= timeitem) {
				foundNode = temp;
				break;
			}
		}
		if (temp == temp->pNext) {
			/* Default this to the last node */
			foundNode = temp;
			break;
		}
		temp = temp->pNext;
	} while (1);


	struct nodelist_s* newItem;
	if (((newItem = ((struct nodelist_s*)calloc(1, sizeof(struct nodelist_s))))!= NULL)) {
		// The new node should point at itself
		newItem->data = node;
		newItem->pNext = newItem;
		newItem->pPrev = newItem;

		if ((foundNode != NULL) && (foundNode != &treenodes)) {
			if (newItem->data->total_time > foundNode->data->total_time) {
				foundNode->pNext = newItem;
				newItem->pPrev = foundNode;
			} else {
				foundNode->pPrev->pNext = newItem;
				newItem->pNext = foundNode;
				newItem->pPrev = foundNode->pPrev;
				foundNode->pPrev = newItem;
			}
		} else {
			// Add the node to the head
			treenodes.pNext = newItem;
			newItem->pPrev = &treenodes;
		}
	}

}
static void
sortTreeData(int* totalTimeLength, int* nameLength, int* calledLength)
{
	node_t *node;
	char temp[MAXLINE];

	treenodes.data = NULL;
	treenodes.pNext = &treenodes;
	treenodes.pPrev = &treenodes;

	RB_FOREACH(node, mytree, &tree) {

		// Need this for intentation purposes
		memset(temp, 0, sizeof(temp));
		sprintf(temp, "%f", node->total_time);
		if ((*totalTimeLength) < (int)strlen(temp)) {
			(*totalTimeLength) = strlen(temp);
		}
		memset(temp, 0, sizeof(temp));
		sprintf(temp, "%d", node->called);
		if ((*calledLength) < (int)strlen(temp)) {
			(*calledLength) = (int)strlen(temp);
		}
		if ((node->name != NULL) && ((*nameLength) < strlen(node->name))) {
			(*nameLength) = strlen(node->name);
		}
		// Add the tree node
		addTreeItem(node);
	}
}

static char* getPaddingDouble(char* buff, int length, float value, int max) {
	int padding = 0;
	int i = 0;

	memset(buff, 0, length);
	sprintf(buff, "%f", value);
	padding = max - strlen(buff);
	if (padding < 0) {
		padding = 0;
	}
	memset(buff, 0, length);
	for (i = 0; i < padding; i++) {
		strcat(buff, " ");
	}

	return buff;
}

static char* getPaddingString(char* buff, int length, char* text, int max) {
	int padding = 0;
	int i = 0;

	padding = max - strlen(text);
	memset(buff, 0, length);
	if (padding < 0) {
		padding = 0;
	}
	for (i = 0; i < padding; i++) {
		strcat(buff, " ");
	}

	return buff;
}
static char* getPaddingLong(char* buff, int length, long value, int max) {
	int padding = 0;
	int i = 0;

	memset(buff, 0, length);
	sprintf(buff, "%ld", value);
	padding = max - strlen(buff);
	if (padding < 0) {
		padding = 0;
	}
	memset(buff, 0, length);
	for (i = 0; i < padding; i++) {
		strcat(buff, " ");
	}

	return buff;
}

static void
linePadding(FILE* file, char* buffer, size_t bufferSize, int paddingWidth, char paddingChar)
{
	int i;
	int max;
	max = (paddingWidth < bufferSize) ? paddingWidth : bufferSize;

	memset(buffer, 0, bufferSize);
	for (i = 0; i < max; i++) {
		buffer[i] = paddingChar;
	}
	strcat(buffer, "\n");
	fputs(buffer, file);
}

void save_values_to_file()
{
	char buffer[MAXLINE];

	char padding1[MAXLINE];
	char padding2[MAXLINE];
	char padding3[MAXLINE];

	char* column1 = "Total Time";
	char* column2 = "Process Name";
	char* column3 = "Num calls";
	char* column4 = "Percent";

	double tmp = 0.0, sum = 0.0;
	struct nodelist_s* temp;
	node_t *node;
	FILE* pfile;
	int totalTimeLength = 0;
	int nameLength = 0;
	int calledLength = 0;
	int tempi;
	int paddingLength;

	pfile = fopen(OUTPUT_FILE, "w");
	if (!pfile) {
		printf("Cannot open %s (error = %d)\n", OUTPUT_FILE, errno);
		exit(EXIT_FAILURE);
	}

	// Sort all the tree data
	sortTreeData(&totalTimeLength, &nameLength, &calledLength);

	// Fix the max column info
	if (totalTimeLength < (int)strlen(column1)) {
		totalTimeLength = (int)strlen(column1);
	}
	if (nameLength < (int)strlen(column2)) {
		nameLength = (int)strlen(column2);
	}
	if (calledLength < (int)strlen(column3)) {
		calledLength = (int)strlen(column3);
	}
	tempi = strlen(column4);
	paddingLength = totalTimeLength+nameLength+calledLength+3/*number of |*/+tempi;

	printf("Saving data to file %s\n", OUTPUT_FILE);

	// Add some table padding
	linePadding(pfile,
		buffer,
		MAXLINE,
		paddingLength,
		'-');

	// Format the column titles for the header
	memset(buffer, 0, MAXLINE);
	snprintf(buffer, MAXLINE, "%s%s|%s%s|%s%s|%s\n",
		column1, getPaddingString(padding1, sizeof(padding1), column1, totalTimeLength),
		column2, getPaddingString(padding2, sizeof(padding2), column2, nameLength),
		column3, getPaddingString(padding3, sizeof(padding3), column3, calledLength),
		column4);
	fputs(buffer, pfile);

	// Add some table padding
	linePadding(pfile,
		buffer,
		MAXLINE,
		paddingLength,
		'-');

	temp = &treenodes;
	do {
		if ((node = temp->data) != NULL) {
			tmp = (node->total_time/(last_timestamp - start_timestamp))*100;
			sum += tmp;
			snprintf(buffer, MAXLINE, "%f%s|%s%s|%d%s|%.3f%c\n",
				node->total_time, getPaddingDouble(padding1, sizeof(padding1), node->total_time, totalTimeLength),
				node->name, getPaddingString(padding2, sizeof(padding2), node->name, nameLength),
				(int)node->called, getPaddingLong(padding3, sizeof(padding3), node->called, calledLength),
				tmp, '%');

			fputs(buffer, pfile);
			memset(buffer, 0, MAXLINE);
		}
		if (temp == temp->pNext) {
			// This is the last line, so write some information here

			// Add some table padding
			linePadding(pfile,
				buffer,
				MAXLINE,
				paddingLength,
				'-');

			snprintf(buffer, MAXLINE, "%s%s|%s%s|%s%s|%.1f%c\n",
				"", getPaddingString(padding1, sizeof(padding1), "", totalTimeLength),
				"", getPaddingString(padding2, sizeof(padding2), "", nameLength),
				"", getPaddingString(padding3, sizeof(padding3), "", calledLength),
				sum, '%');
			fputs(buffer, pfile);

			// This is the last line, so write some information here
			// Add some table padding
			linePadding(pfile,
				buffer,
				MAXLINE,
				paddingLength,
				'-');

			memset(buffer,0,MAXLINE);
			sprintf(buffer,"Total time: %.1f seconds\n", (last_timestamp - start_timestamp));
			fputs(buffer,pfile);
			break;
		}
		temp = temp->pNext;
	} while (1);

	printf("%ld bytes written to file %s\n", ftell(pfile), OUTPUT_FILE);
	fclose(pfile);
}

void del()
{
	struct nodelist_s* temp;

	// Remove the allocated linked list
	temp = (&treenodes)->pNext;
	do {
		struct nodelist_s* old;
		if (temp == temp->pNext) {
			break;
		}
		old = temp;
		temp = temp->pNext;
		free(old);
	} while (1);

	if (temp != &treenodes) {
		free(temp);
	}

free(last_to_run.name);
}


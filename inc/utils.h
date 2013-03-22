#ifndef _UTILS_H_
#define _UTILS_H_
void init();
void add_to_list(char* str,double time);
void del();
void save_values_to_file();

#define MAXLINE		250
#define OUTPUT_FILE	"output.log"

#ifndef TARGET
	#define TARGET	1
#endif

typedef unsigned int uint32;
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include "include/shell.h"
#include "include/command.h"
#include "include/builtin.h"
#include "include/resource.h"
#include "include/schedule.h"
struct schedule_t *scheduler;
bool *resource;
enum AlgorithmType type;
int main(int argc, char *argv[])
{

	scheduler=NULL ;//initialize to NULL

	resource=(bool*)malloc(8*sizeof(bool));
	for(int i=0;i<8;i++){
		resource[i] = true;
	}

	if(!strcmp("FCFS",argv[1])){
		type = FCFS;
	}else if(!strcmp("RR", argv[1])){
		type = RR;
	}else if(!strcmp("PP", argv[1])){
		type = PP;
	}else{
		printf("algorithm type error");
		return 0;
	}

	history_count = 0;
	for (int i = 0; i < MAX_RECORD_NUM; ++i)
    	history[i] = (char *)malloc(BUF_SIZE * sizeof(char));

	shell();

	for (int i = 0; i < MAX_RECORD_NUM; ++i)
    	free(history[i]);

	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include "../include/builtin.h"
#include "../include/schedule.h"
#include "../include/handlesignal.h"
#include "../include/shell.h"
#include "../include/function.h"
struct schedule_t *scheduler;
struct itimerval value, ovalue;
sigset_t x;
ucontext_t idle_ctx;

int help(char **args)
{
	int i;
    printf("--------------------------------------------------\n");
  	printf("My Little Shell!!\n");
	printf("The following are built in:\n");
	for (i = 0; i < num_builtins(); i++) {
    	printf("%d: %s\n", i, builtin_str[i]);
  	}
	printf("%d: replay\n", i);
    printf("--------------------------------------------------\n");
	return 1;
}

int cd(char **args)
{
	if (args[1] == NULL) {
    	fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  	} else {
    	if (chdir(args[1]) != 0)
      		perror("lsh");
	}
	return 1;
}

int echo(char **args)
{
	bool newline = true;
	for (int i = 1; args[i]; ++i) {
		if (i == 1 && strcmp(args[i], "-n") == 0) {
			newline = false;
			continue;
		}
		printf("%s", args[i]);
		if (args[i + 1])
			printf(" ");
	}
	if (newline)
		printf("\n");

	return 1;
}

int exit_shell(char **args)
{
	return 0;
}

int record(char **args)
{
	if (history_count < MAX_RECORD_NUM) {
		for (int i = 0; i < history_count; ++i)
			printf("%2d: %s\n", i + 1, history[i]);
	} else {
		for (int i = history_count % MAX_RECORD_NUM; i < history_count % MAX_RECORD_NUM + MAX_RECORD_NUM; ++i)
			printf("%2d: %s\n", i - history_count % MAX_RECORD_NUM + 1, history[i % MAX_RECORD_NUM]);
	}
	return 1;
}

bool isnum(char *str)
{
	for (int i = 0; i < strlen(str); ++i) {
    	if(str[i] >= 48 && str[i] <= 57)
			continue;
        else
		    return false;
  	}
  	return true;
}

int mypid(char **args)
{
	char fname[BUF_SIZE];
	char buffer[BUF_SIZE];
	if(strcmp(args[1], "-i") == 0) {

	    pid_t pid = getpid();
	    printf("%d\n", pid);
	
	} else if (strcmp(args[1], "-p") == 0) {
	
		if (args[2] == NULL) {
      		printf("mypid -p: too few argument\n");
      		return 1;
    	}

    	sprintf(fname, "/proc/%s/stat", args[2]);
    	int fd = open(fname, O_RDONLY);
    	if(fd == -1) {
      		printf("mypid -p: process id not exist\n");
     		return 1;
    	}

    	read(fd, buffer, BUF_SIZE);
	    strtok(buffer, " ");
    	strtok(NULL, " ");
	    strtok(NULL, " ");
    	char *s_ppid = strtok(NULL, " ");
	    int ppid = strtol(s_ppid, NULL, 10);
    	printf("%d\n", ppid);
	    
		close(fd);

  	} else if (strcmp(args[1], "-c") == 0) {

		if (args[2] == NULL) {
      		printf("mypid -c: too few argument\n");
      		return 1;
    	}

    	DIR *dirp;
    	if ((dirp = opendir("/proc/")) == NULL){
      		printf("open directory error!\n");
      		return 1;
    	}

    	struct dirent *direntp;
    	while ((direntp = readdir(dirp)) != NULL) {
      		if (!isnum(direntp->d_name)) {
        		continue;
      		} else {
        		sprintf(fname, "/proc/%s/stat", direntp->d_name);
		        int fd = open(fname, O_RDONLY);
        		if (fd == -1) {
          			printf("mypid -p: process id not exist\n");
          			return 1;
        		}

        		read(fd, buffer, BUF_SIZE);
        		strtok(buffer, " ");
        		strtok(NULL, " ");
        		strtok(NULL, " ");
		        char *s_ppid = strtok(NULL, " ");
		        if(strcmp(s_ppid, args[2]) == 0)
		            printf("%s\n", direntp->d_name);

        		close(fd);
     		}
	   	}
    	
		closedir(dirp);
	
	} else {
    	printf("wrong type! Please type again!\n");
  	}
	
	return 1;
}

const char *function_str[] = {
 	"task1",
 	"task2",
	"task3",
	"task4",
	"task5",
	"task6",
	"task7",
	"task8",
	"task9",
	"test_exit",
	"test_sleep",
	"test_resource1",
	"test_resource2",
	"idle",
};
const void (*task_func[]) () = {
	&task1,
	&task2,
	&task3,
	&task4,
  	&task5,
	&task6,
	&task7,
	&task8,
	&task9,
	&test_exit,
	&test_sleep,
	&test_resource1,
	&test_resource2,
	&idle,
};



int add(char **args)
{
	//if scheduler has not initialize yet
	if(scheduler==NULL) ininialize_sched();
	char function_name[16];
	memset(function_name, '\0', sizeof(function_name));
	int priority=-1;

	strcpy(function_name, args[2]);
	if(type==PP){
		priority = atoi(args[3]);
	}


	// start to create uthread
	int thread_id=-1;
	for(int i=0;i<num_tasks();i++){
		if (strcmp(function_name, function_str[i]) == 0){
			thread_id = uthread_create(*task_func[i],NULL,priority, args[1]);
			if(type==PP){
				Queue_Push_priority(scheduler->readyQ,thread_id);
				
			}else {Queue_Push(scheduler->readyQ, thread_id);}
			break;
		}
		if(i==num_tasks()-1){
			printf("task name error");
			return 1;
		}
	}
	
	
	printf("Task %s is ready.\n", args[1]);
	fflush(stdout);
	return 1;
}

int del(char **args)
{
	for(int i=0; i<scheduler->max_index;i++){
		if(strcmp(args[1], scheduler->threads[i].name)==0){
			uthread_t *t = &scheduler->threads[i];
			t->state = TERMINATED;
			printf("Task %s is killed.\n", args[1]);
			break;
		}
	}
	return 1;
}

int ps(char **args)
{
	// if(type==PP) printf(" TID|\tname|\tstate| running| waiting| turnaround| resources| priority\n");
	// else printf(" TID|\tname|\tstate| running| waiting| turnaround| resources\n");
	if(type==PP) printf("%5s|%8s|%12s|%10s|%10s|%11s|%10s|%10s\n","TID","name","state","running","waiting", "turnaround","resources","priority");
	else printf("%5s|%8s|%12s|%10s|%10s|%11s|%10s\n","TID","name","state","running","waiting", "turnaround","resources");
	printf("-----------------------------------------------------------------------------------\n");
	for(int i=0;i<scheduler->max_index;i++){
		uthread_t *t = &scheduler->threads[i];
		if(t->state!=FREE){

			printf("%5d|%8s|%12s|%10d|%10d",
				t->id+1, 
				t->name, 
				enum_str_mapping(t->state),
				t->runningTime, 
				t->waitingTime
			);
			if(t->state==TERMINATED)	printf("|%11d|",t->turnaroundTime);
			else{
				printf("|%11s|", "none");
			}
			if(t->resourceQ!=NULL){
				QueueNode *n = t->resourceQ->front;
				char buffer[16];
				char res[16];
				memset(buffer, '\0', sizeof(buffer));
				memset(res, '\0', sizeof(res));
				if(Queue_IsEmpty(t->resourceQ)==false){
					for(;n!=NULL;n=n->next){
						sprintf(buffer, " %d", n->data);
						strcat(res,buffer);
					}
					printf("%10s", res);
				}else printf("%10s","none");
			}else printf("%10s","none");

			if(type==PP)	printf("|%10d\n",t->priority);
			else{
				printf("\n");
			}

		}
	}
	return 1;
}



int start(char **args)
{
	if(scheduler->isPause==true){
		scheduler->isPause = false;
		// printf("restore\n");
		signal(SIGTSTP, sigroutine);    //make sure signal configuration
		signal(SIGVTALRM, sigroutine);
		setcontext(&(scheduler->tstp_ctx));
	}
	printf("Start simulation.\n");
	sigemptyset (&x);
	sigaddset(&x, SIGVTALRM);
	signal(SIGVTALRM, sigroutine);
	signal(SIGTSTP, sigroutine);
	value.it_value.tv_sec = 0;
	value.it_value.tv_usec = 10000;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = 10000;
	setitimer(ITIMER_VIRTUAL, &value, &ovalue);
	Queue *readyQ = scheduler->readyQ;

	while(1){
		if(Queue_IsEmpty(readyQ)==false){
			
			sigprocmask(SIG_BLOCK, &x, NULL);
			int id = Queue_front(readyQ);	//the first element in ready queue can get cpu
			scheduler->running_thread = id;
			uthread_t *t = &scheduler->threads[id];
			if(t->state==TERMINATED){
				
				Queue_Pop(readyQ);
				sigprocmask(SIG_UNBLOCK, &x, NULL);
				continue;
			}
			t->state = RUNNING;
			printf("Task %s is running.\n", t->name);
			fflush(stdout);
			sigprocmask(SIG_UNBLOCK, &x, NULL);
			swapcontext(&(scheduler->main), &(t->ctx));
			fflush(stdout);
		}else if(Queue_IsEmpty(scheduler->waitQ)==false){	//no one is ready, but someone is still waiting
			printf("CPU idle.\n");
			scheduler->idle=true;
			swapcontext(&(scheduler->main), &(idle_ctx));
			scheduler->idle=false;
		}else{	//no one is waiting and no one is in ready queue 
			if(Queue_IsEmpty(readyQ)==false) continue;	//check again
			break;
		}
		// printf("check\n");
	}
	
	value.it_value.tv_sec = 0;
	value.it_value.tv_usec = 0;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = 0;
	setitimer(ITIMER_VIRTUAL, &value, &ovalue);
	signal(SIGTSTP, SIG_DFL);
	printf("Simulation over.\n");
	fflush(stdout);
	return 1;
}


const char *builtin_str[] = {
 	"help",
 	"cd",
	"echo",
 	"exit",
 	"record",
	"mypid",
	"add",
	"del",
	"ps",
	"start",
};

const int (*builtin_func[]) (char **) = {
	&help,
	&cd,
	&echo,
	&exit_shell,
  	&record,
	&mypid,
	&add,
	&del,
	&ps,
	&start
};

int num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}

int num_tasks() {
	return sizeof(function_str) / sizeof(char *);
}
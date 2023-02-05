#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/schedule.h"
#include "../include/queue.h"
#include "../include/shell.h"
#include "../include/function.h"
struct schedule_t *scheduler;
ucontext_t shell_ctx;
ucontext_t idle_ctx;
enum AlgorithmType type;

void ininialize_sched(){
	scheduler = (schedule_t*)malloc(sizeof(schedule_t));
	scheduler->running_thread  = -1;
	scheduler->max_index = 0;
	scheduler->stack = (char*)malloc(sizeof(char)* DEFAULT_STACK_SZIE);
	scheduler->threads = (uthread_t*)malloc(MAX_UTHREAD_SIZE*sizeof(uthread_t));
	for(int i=0;i<MAX_UTHREAD_SIZE;i++){
		scheduler->threads[i].state = FREE;
		scheduler->threads[i].stack = (char*)malloc(sizeof(char)*DEFAULT_STACK_SZIE);
	}

	scheduler->idle=false;
	scheduler->ispreempt = false;
	scheduler->isPause = false;
	scheduler->func = shell;
	scheduler->readyQ = Queue_new();
	scheduler->waitQ = Queue_new();
	getcontext(&(shell_ctx));
	getcontext(&(idle_ctx));
	shell_ctx.uc_stack.ss_sp = scheduler->stack;
    shell_ctx.uc_stack.ss_size = DEFAULT_STACK_SZIE;
    shell_ctx.uc_stack.ss_flags = 0;
    shell_ctx.uc_link = &(scheduler->main);
	idle_ctx.uc_stack.ss_sp = scheduler->stack;
    idle_ctx.uc_stack.ss_size = DEFAULT_STACK_SZIE;
    idle_ctx.uc_stack.ss_flags = 0;
    idle_ctx.uc_link = &(scheduler->main);
	makecontext(&(shell_ctx),(void (*)())(scheduler->func),0);
	makecontext(&(idle_ctx),(void (*)())(idle),0);
}

char* enum_str_mapping(enum ThreadState s){
	if(s==READY){
		return "READY";
	}else if(s==TERMINATED){
		return "TERMINATED";
	}else if(s==WAITING){
		return "WAITING";
	}else if(s==RUNNING){
		return "RUNNING";
	}
	return "something error";
}


int uthread_create(Fun func,void *arg, int priority, char *name){
	int id = 0;
    for(id = 0; id < scheduler->max_index; ++id ){
        if(scheduler->threads[id].state == FREE){
            break;
        }
    }
	if (id == scheduler->max_index) {
        scheduler->max_index++;
    }

    uthread_t *t = &(scheduler->threads[id]);
	strcpy(t->name, name);
	t->id = id;
    t->state = READY;
    t->func = func;
	t->sleeptime = 0;
	t->worktime = 0;
	t->waitingTime = 0;
	t->turnaroundTime = 0;
	t->runningTime = 0;
	t->isSleep = false;
	t->expired = false;
    t->targetSleepTime = 0;
	t->resource_count = 0;
	t->resource_list = NULL;
	t->res_all_avaliable = false;
	t->resourceQ = NULL;
	t->priority = priority;
	if(arg!=NULL){
    	t->arg = arg;
	}

    getcontext(&(t->ctx));
    
    t->ctx.uc_stack.ss_sp = t->stack;
    t->ctx.uc_stack.ss_size = DEFAULT_STACK_SZIE;
    t->ctx.uc_stack.ss_flags = 0;
    t->ctx.uc_link = &(scheduler->main);
    
	makecontext(&(t->ctx),(void (*)())(t->func),0);

    return id;
}

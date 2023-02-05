#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/handlesignal.h"
#include "../include/schedule.h"
#include "../include/resource.h"
#include "../include/shell.h"
#include "../include/queue.h"

struct schedule_t *scheduler;
struct itimerval value, ovalue;
ucontext_t shell_ctx;
enum AlgorithmType type;
sigset_t x;
bool goback;

void FCFShandler(){
	// printf("signal handle type: %d\n", type);
		QueueNode *n ;
		for(n=scheduler->waitQ->front; n!=NULL ;){
			uthread_t *t = &scheduler->threads[n->data];
			n=n->next;
			t->turnaroundTime++;
			/*check the threads in waiting queue*/
			// printf("check %s\n", t->name);
			if(t->isSleep==true){
				// printf("thread :%s is waiting and sleeping\n", t->name);
				t->sleeptime+=1;
				if(t->sleeptime==t->targetSleepTime){	//wake up
					// printf("wake up thread name:%s\n", t->name);
					t->isSleep=false;
					t->state = READY;
					t->sleeptime = 0;
					t->targetSleepTime = 0;
					Queue_Push(scheduler->readyQ, t->id);
					Queue_remove(scheduler->waitQ, t->id);
					goback=true;
					continue;
				}
				
			}else{	//not sleeping, but in waiting queue
				/*check resource*/
				// printf("checking resource for thread :%s\n", t->name);
				t->res_all_avaliable = true;
				for(int j=0;j<t->resource_count;j++){
					int res_id = t->resource_list[j];
					if(resource[res_id]==true) {
						continue;   //keep checking
					}else{
						t->res_all_avaliable = false;
						break;	//if any one of resource is un avaliable then break
					}
				}
				if(t->res_all_avaliable){
					t->state = READY;
					Queue_Push(scheduler->readyQ, t->id);
					Queue_remove(scheduler->waitQ, t->id);
					goback=true;
					continue;
				}
			}
			
		}
}

void PPhandler(){
		QueueNode *n ;
		for(n=scheduler->waitQ->front; n!=NULL ; ){
			uthread_t *t = &scheduler->threads[n->data];
			n=n->next;
			t->turnaroundTime++;
			/*check the threads in waiting queue*/
			if(t->isSleep==true){
				// printf("thread :%s is waiting and sleeping\n", t->name);
				t->sleeptime+=1;
				if(t->sleeptime==t->targetSleepTime){	//wake up
					// printf("wake up thread name:%s\n", t->name);
					t->isSleep=false;
					t->state = READY;
					t->sleeptime = 0;
					t->targetSleepTime = 0;
					Queue_Push_priority(scheduler->readyQ, t->id);
					Queue_remove(scheduler->waitQ, t->id);
					goback=true;

					if(scheduler->running_thread!=-1){
						if( t->priority < scheduler->threads[scheduler->running_thread].priority ){
							scheduler->ispreempt = true;
							// printf("preempt happend!!!\n");
						}
					}
				}
				
			}else{	//not sleeping, but in waiting queue
				/*check resource*/
				// printf("checking resource for thread :%s\n", t->name);
				t->res_all_avaliable = true;
				for(int j=0;j<t->resource_count;j++){
					int res_id = t->resource_list[j];
					if(resource[res_id]==true) {
						continue;   //keep checking
					}else{
						t->res_all_avaliable = false;
						break;	//if any one of resource is un avaliable then break
					}
				}
				if(t->res_all_avaliable){

					if(scheduler->running_thread!=-1){	//if someone is runnign on cpu
						//if new task has higher priority than the task running now
						if( t->priority < scheduler->threads[scheduler->running_thread].priority ){
							scheduler->ispreempt = true;
							// printf("preempt happend!!!\n");
						}
					}
					t->state = READY;
					Queue_Push_priority(scheduler->readyQ, t->id);
					Queue_remove(scheduler->waitQ, t->id);
					goback=true;		
							

				}
			}
		}
		if(scheduler->ispreempt){
			int running_id = scheduler->running_thread;
			scheduler->ispreempt=false;
			if(running_id !=-1){
				uthread_t *t = &scheduler->threads[running_id];
				scheduler->running_thread = -1;
				scheduler->threads[scheduler->running_thread].state=READY;
				sigprocmask(SIG_UNBLOCK, &x, NULL);
				swapcontext(&(t->ctx),&(scheduler->main));	//exit now and reschedule
				sigprocmask(SIG_BLOCK, &x, NULL);
			}
		}

}

void RRhandler(){
		QueueNode *n ;
		for(n=scheduler->waitQ->front; n!=NULL ;){
			uthread_t *t = &scheduler->threads[n->data];
			n=n->next;
			t->turnaroundTime++;

			/*check the threads in waiting queue*/
			if(t->isSleep==true){
				// printf("thread :%s is waiting and sleeping\n", t->name);
				t->sleeptime+=1;
				if(t->sleeptime==t->targetSleepTime){	//wake up
					// printf("wake up thread name:%s\n", t->name);
					t->isSleep=false;
					t->state = READY;
					t->sleeptime = 0;
					t->targetSleepTime = 0;
					Queue_Push(scheduler->readyQ, t->id);
					Queue_remove(scheduler->waitQ, t->id);
					goback=true;
				}
				
			}else{	//not sleeping, but in waiting queue
				/*check resource*/
				t->res_all_avaliable = true;
				for(int j=0;j<t->resource_count;j++){
					int res_id = t->resource_list[j];
					if(resource[res_id]==true) {
						continue;   //keep checking
					}else{
						t->res_all_avaliable = false;
						break;	//if any one of resource is un avaliable then break
					}
				}
				if(t->res_all_avaliable){

					t->state = READY;
					Queue_Push(scheduler->readyQ, t->id);
					Queue_remove(scheduler->waitQ, t->id);
					goback=true;
				}
			}
		}
		/*check if need to reschedule*/
		int running_id = scheduler->running_thread;
		if(running_id!=-1){		//someine is running
			uthread_t *t = &scheduler->threads[running_id];
			t->worktime+=1;
			if(t->worktime==3){	//30ms
				t->worktime=0;	//reset worktime
				t->expired = true;	//run out of time quantum
			}
			if(t->expired==true){
				t->expired=false;
				t->state=READY;
				scheduler->running_thread = -1;
				Queue_Pop(scheduler->readyQ); //remove the first thread in readyQ
				Queue_Push(scheduler->readyQ, running_id);
				// printf("task %s run out of time, enter ready queue\n", t->name);
				sigprocmask(SIG_UNBLOCK, &x, NULL);
				swapcontext(&(t->ctx),&(scheduler->main));
				sigprocmask(SIG_BLOCK, &x, NULL);
			}
		}
}



/*handle the signal*/
void alarmHandler(){
	// printf("---------Catch a signal -- SIGVTALRM--------------\n");	
	if(scheduler->running_thread!=-1){
		uthread_t *t = &scheduler->threads[scheduler->running_thread];
		t->runningTime++;
		t->turnaroundTime++;
	}

	//for readyQ calculate running time
	QueueNode *n ;
	//the first one is the running task so skip it
	if(scheduler->readyQ->front!=NULL){
		if(scheduler->readyQ->front->next!=NULL){
			n=scheduler->readyQ->front->next;
			for(; n!=NULL ;){
				uthread_t *t = &scheduler->threads[n->data];
				t->turnaroundTime++;
				t->waitingTime++;
				n=n->next;
			}
		}
	}



	if(type==FCFS){
		FCFShandler();
	}else if(type==RR){
		RRhandler();
	}else if(type==PP){
		PPhandler();
	}
	signal(SIGVTALRM, sigroutine);    //make sure signal configuration
}

void TSTPHandler(){
	printf("Catch a signal -- SIGTSTP \n");
	/*pause the simulation*/
	scheduler->isPause = true;
	signal(SIGTSTP, SIG_DFL);
	swapcontext(&(scheduler->tstp_ctx), &(shell_ctx));
	signal(SIGTSTP, sigroutine);    //make sure signal configuration
}

void sigroutine(int signo){
	fflush(stdout);
	sigprocmask(SIG_BLOCK, &x, NULL);
	goback=false;
	switch(signo){
		case SIGTSTP:
			TSTPHandler();
			break;
		case SIGVTALRM:
			alarmHandler();
			break;
	}
	// printf("running id after sigroutine %d\n", scheduler->running_thread);
	sigprocmask(SIG_UNBLOCK, &x, NULL);
	if(scheduler->idle==true && goback==true) setcontext(&(scheduler->main));
	return;
}


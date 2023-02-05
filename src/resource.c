#include <stdlib.h>
#include <stdio.h>
#include "../include/resource.h"
#include "../include/queue.h"
#include "../include/builtin.h"
#include "../include/schedule.h"
#include "../include/handlesignal.h"
struct schedule_t *scheduler;
bool *resource;
sigset_t x;

void get_resources(int count, int *resources)
{
    // printf("want to get resource\n");

    int running_thread_id = scheduler->running_thread;
    uthread_t *t = &scheduler->threads[running_thread_id];
    t->resource_count  = count;
    if(t->resource_list==NULL) t->resource_list = malloc(sizeof(int)*count);
    if(t->resourceQ==NULL)  t->resourceQ = Queue_new();
    while(1){
        t->res_all_avaliable = true;    //assume true
        int res_id;
        for(int i=0;i<count;i++){
            t->resource_list[i] = resources[i]; //record how many resource need for this thread
            res_id = resources[i];
            if(resource[res_id]==true) {
                continue;   //keep checking
            }else{
                t->res_all_avaliable = false;
            }
        }

    
        if(t->res_all_avaliable){
            /*all resource are avaliable*/
            sigprocmask(SIG_BLOCK, &x, NULL);
            for(int i=0;i<count;i++){
                res_id = resources[i];
                resource[res_id]=false; //get resource, set the flag to false
                printf("Task %s gets resource %d.\n", t->name, resources[i]);
                Queue_Push(t->resourceQ, resources[i]);
            }
            sigprocmask(SIG_UNBLOCK, &x, NULL);
            break;
        }else{
            /*some resource is not avaliable*/
            // printf("id: %d\n", running_thread_id);
            printf("Task %s is waiting resource.\n", t->name);
            sigprocmask(SIG_BLOCK, &x, NULL);
            t->state = WAITING;
            scheduler->running_thread = -1;
            Queue_Push(scheduler->waitQ, t->id);
            Queue_Pop(scheduler->readyQ); //remove the first thread in readyQ
            sigprocmask(SIG_UNBLOCK, &x, NULL);
            swapcontext(&(t->ctx),&(scheduler->main));
        }
    }


    return;
}

void release_resources(int count, int *resources)
{
    sigprocmask(SIG_BLOCK, &x, NULL);
    int running_thread_id = scheduler->running_thread;
    uthread_t *t = &scheduler->threads[running_thread_id];
    int res_id;
    for(int i=0;i<count;i++){
        res_id  = resources[i];
        resource[res_id] = true;
        printf("Task %s releases resource %d.\n", t->name, resources[i]);
        Queue_remove(t->resourceQ, resources[i]);
    }
    sigprocmask(SIG_UNBLOCK, &x, NULL);
}

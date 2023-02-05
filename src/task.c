#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "../include/task.h"
#include "../include/resource.h"
#include "../include/schedule.h"
#include "../include/handlesignal.h"
struct schedule_t *scheduler;
sigset_t x;

void task_sleep(int ms)
{
    sigprocmask(SIG_BLOCK, &x, NULL);
    int id = scheduler->running_thread;
    // printf("running id in sleep: %d", id);
    uthread_t *t = &scheduler->threads[id];
    t->targetSleepTime = ms;
    Queue_Push(scheduler->waitQ, id);
    scheduler->running_thread = -1;
    Queue_Pop(scheduler->readyQ);
    t->isSleep = true;
    t->state = WAITING;
    printf("Task %s goes to sleep.\n", t->name);
    sigprocmask(SIG_UNBLOCK, &x, NULL);
    swapcontext(&(t->ctx), &(scheduler->main)); //record in t->ctx, run scheduler->main

}

void task_exit()
{
    sigprocmask(SIG_BLOCK, &x, NULL);
    int id = scheduler->running_thread;
    uthread_t *t = &scheduler->threads[id];
    scheduler->running_thread = -1;
    Queue_Pop(scheduler->readyQ); //pop out the first element, which is using cpu
    printf("Task %s has terminated.\n", t->name);
    t->state = TERMINATED;
    sigprocmask(SIG_UNBLOCK, &x, NULL);
    setcontext(&(scheduler->main));
}

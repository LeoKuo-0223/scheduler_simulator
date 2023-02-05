#ifndef SCHEDULE_H
#define SCHEDULE_H
#include <ucontext.h>
#include <stdbool.h>
#include "../include/queue.h"

#define DEFAULT_STACK_SZIE (1024*128)
#define MAX_UTHREAD_SIZE   64
enum ThreadState {
    TERMINATED,
    RUNNING,
    WAITING,
    READY,
    FREE,
};

enum AlgorithmType {
    FCFS ,
    RR ,
    PP
};
struct schedule_t;

typedef void (*Fun)();

typedef struct uthread_t
{
    int id;
    ucontext_t ctx;
    // ucontext_t reschedule_ctx;  //temparily remember the original location, and jump to reschedule the cpu
    Fun func;
    void *arg;
    enum ThreadState state;
    char *stack;
    int priority;
    char name[16];
    int sleeptime; /*10 msec resolution*/
    int targetSleepTime;
    bool isSleep;
    int resource_count;
    int *resource_list;
    bool res_all_avaliable;
    bool expired;
    int worktime;   //for round robin mode record the quantum time
    int runningTime;
    int turnaroundTime;
    int waitingTime;
    Queue *resourceQ;
}uthread_t;

typedef struct schedule_t
{
    ucontext_t main;
    ucontext_t tstp_ctx;
    char *stack;
    Fun func;
    int running_thread;
    uthread_t *threads;
    int max_index; // 曾經使用到的最大的index + 1
    Queue *readyQ ;
    Queue *waitQ ;
    bool isPause;
    bool ispreempt;
    bool idle;
}schedule_t;


extern bool goback;
/*declaration of scheduler*/
extern struct schedule_t *scheduler;

/*algorithm type*/
extern enum AlgorithmType type;

/*record the context in shell mode*/
extern ucontext_t shell_ctx;
extern ucontext_t idle_ctx;
/*return the index of the created thread in schedule*/
int  uthread_create(Fun func,void *arg, int priority, char *name);

char* enum_str_mapping(enum ThreadState s);

void ininialize_sched();

#endif
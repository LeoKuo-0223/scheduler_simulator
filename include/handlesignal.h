#ifndef HANDLESIGNAL_H
#define HANDLESIGNAL_H
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <ucontext.h>
#include <stddef.h>

/*timer value*/
extern struct itimerval value, ovalue;


extern sigset_t x;

/*signal*/
void sigroutine(int signo);
void alarmHandler();
void TSTPHandler();
void FCFShandler();
void PPhandler();
void RRhandler();
#endif
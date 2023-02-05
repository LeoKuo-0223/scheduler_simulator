#ifndef QUEUE_H
#define QUEUE_H
#include <stdbool.h>

typedef struct QueueNode{
    int data;
    struct QueueNode *next;
    struct QueueNode *last;
    // QueueNode():data(0),next(0){};
    // QueueNode(int x):data(x),next(0){};
}QueueNode;

typedef struct Queue{
    QueueNode *front;
    QueueNode *back;
    int size;
    // int *queue;
    // int front;
    // int rear;
    // int num_cmd;
}Queue;

Queue* Queue_new();
void Queue_Push(Queue *q, int x);
int Queue_Pop(Queue *q);
bool Queue_IsEmpty(Queue *q);
int Queue_front(Queue *q);
void Queue_Push_priority(Queue *q, int x);
void Queue_remove(Queue *q, int x);
// int getBack(Queue *q);

// Queue* Queue_new(int capacity);
// void Queue_Push(Queue *q, int x);
// int Queue_Pop(Queue *q);
// bool Queue_IsEmpty(Queue *q);
// int Queue_front(Queue *q);
// void Queue_sort(Queue *q);
// int cmp(const void *a, const void *b);
#endif
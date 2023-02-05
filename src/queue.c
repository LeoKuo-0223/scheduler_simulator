#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "../include/queue.h"
#include "../include/schedule.h"
struct schedule_t *scheduler;

Queue* Queue_new(){ //constructor for queue
    Queue *q = (Queue*)malloc(sizeof(Queue));
    q->front = NULL;
    q->back = NULL;
    q->size = 0;
    return q;
}

bool Queue_IsEmpty(Queue *q){       //check the queue is empty or not
    // printf("enter is empty\n");
    return q->size==0;
}

void Queue_Push(Queue *q, int x){      //add new element to queue
    QueueNode *n = (QueueNode*)malloc(sizeof(QueueNode));
    n->data = x;
    n->next = NULL;
    if (Queue_IsEmpty(q)) {
        n->last = NULL;
        q->front = n;
        q->back = q->front;
        q->size++;
        return;
    }
    n->last = q->back;
    q->back->next = n;
    q->back = n;
    q->size++;


    // n = q->front;
    // if(n!=NULL){
    //     // printf("after push: \n");
    //     while(1){
    //         if(n==NULL) break;
    //         uthread_t *t =&scheduler->threads[n->data];
    //         printf("%s->", t->name);
    //         n = n->next;
    //     }
    //     printf("push over\n");
    // }
    return;
}

int Queue_Pop(Queue *q){       //delete element from queue
    if(Queue_IsEmpty(q)){
        printf("Queue is empty!\n");
        return -1;
    }
    int delete_value;
    QueueNode *delete_node = q->front;
    delete_value = delete_node->data;
    if(delete_node->next!=NULL){
        delete_node->next->last = NULL;
        q->front = delete_node->next;
    }else{
        q->front = NULL;
        q->back = NULL;
    }

    free(delete_node);
    q->size--;

    // QueueNode *n;
    // n = q->front;
    // if(n!=NULL){
    //     printf("after pop: \n");
    //     while(1){
    //         if(n==NULL) break;
    //         uthread_t *t =&scheduler->threads[n->data];
    //         printf("%s->", t->name);
    //         n = n->next;
    //     }
    //     printf("pop over\n");
    // }
    return delete_value;
}

int Queue_front(Queue *q){
    if(Queue_IsEmpty(q)){
        printf("Queue is empty!\n");
        return -1;
    }
    return q->front->data;
}


void Queue_Push_priority(Queue *q, int x){
    QueueNode *n = (QueueNode*)malloc(sizeof(QueueNode));
    n->data = x;
    n->next = NULL;
    n->last = NULL;
    if (Queue_IsEmpty(q)) {
        q->front = n;
        q->back = q->front;
        q->size++;
        return;
    }
    
    uthread_t *newt = &scheduler->threads[x];
    QueueNode *tmp = q->front;
    while(1){
        if(tmp==NULL) break;
        uthread_t *oldt = &scheduler->threads[tmp->data]; 
        if(newt->priority>oldt->priority){
            tmp = tmp->next;
        }else break;
    }
    //if priority is the smallest one
    if(tmp==NULL) Queue_Push(q, x); //push to the last one
    else{
        if(tmp==q->front){
            q->front = n;
            n->next = tmp;
            n->last = NULL;
            tmp->last = n;
        }else{
            n->next = tmp;
            n->last = tmp->last;
            tmp->last->next = n;
            tmp->last = n;
        }
        q->size++;

    }
    // QueueNode *node;
    // node = q->front;
    // while(1){
    //     if(node==NULL) break;
    //     uthread_t *t =&scheduler->threads[node->data];
    //     printf("%s->", t->name);
    //     node = node->next;
    // }
    // printf("\n");
    return;
}

void Queue_remove(Queue *q, int x){
    // printf("enter remove\n");
    if(Queue_IsEmpty(q)){
        printf("Queue is empty!!\n");
        return;
    }
    // QueueNode *n;
    // n = q->front;
    // while(1){
    //     if(n==NULL) break;
    //     uthread_t *t =&scheduler->threads[n->data];
    //     printf("%s->", t->name);
    //     n = n->next;
    // }
    // printf("\n");

    QueueNode *node = q->front;
    while(1){
        if(node!=NULL){
            if(node->data==x){
               break;
            }
        }else{
            printf("Not Found!!\n");
            return;
        }
        node = node->next;
    }
    // printf("find target\n");
    if(node->last==NULL&&node->next==NULL){
        q->front = NULL;
        q->back = NULL;
    }else if(node->next==NULL){
        q->back=node->last;
        q->back->next = NULL;
    }else if(node->last==NULL){
        q->front = node->next;
        q->front->last = NULL;
    }else{  //in the between of nodes
        node->last->next = node->next;
        node->next->last = node->last;
    }
    free(node);
    q->size--;
    // printf("leaving remove\n");
    return;

}
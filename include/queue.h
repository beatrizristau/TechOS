#ifndef QUEUE_H
#define QUEUE_H

#include "pcb.h"

typedef struct {
    int   count;
    PCB  *head;
    PCB  *tail;
} Queue;

extern Queue g_ready_queue;
extern Queue g_blocked_queue;
extern Queue g_suspended_ready_queue;
extern Queue g_suspended_blocked_queue;

void init_queues(void);
void enqueue_ready(PCB *p);
void enqueue_blocked(PCB *p);
void enqueue_suspended_ready(PCB *p);
void enqueue_suspended_blocked(PCB *p);
void dequeue(Queue *q, PCB *p);
void dump_queue(const char *title, Queue *q);
void cleanup_queue(Queue *q);

#endif

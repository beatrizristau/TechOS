#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

Queue g_ready_queue;
Queue g_blocked_queue;
Queue g_suspended_ready_queue;
Queue g_suspended_blocked_queue;

/**
 * @brief Initializes the ready and blocked queues.
 * Sets the count to 0 and head/tail pointers to NULL.
 */
void init_queues(void) {
    g_ready_queue.count = 0; g_ready_queue.head = g_ready_queue.tail = NULL;
    g_blocked_queue.count = 0; g_blocked_queue.head = g_blocked_queue.tail = NULL;
    g_suspended_ready_queue.count = 0; g_suspended_ready_queue.head = g_suspended_ready_queue.tail = NULL;
    g_suspended_blocked_queue.count = 0; g_suspended_blocked_queue.head = g_suspended_blocked_queue.tail = NULL;
}

/**
 * @brief Enqueues a PCB into the ready queue in descending priority order.
 * If the queue is empty, the PCB becomes both head and tail.
 * Otherwise, it finds the correct position based on priority.
 * @param p Pointer to the PCB to be enqueued.
 */
void enqueue_ready(PCB *p) {
    // insert by descending priority
    Queue *q = &g_ready_queue;
    if (!q->head) {
        q->head = q->tail = p;
        p->next = p->prev = NULL;
    } else {
        PCB *cur = q->head;
        while (cur && cur->priority >= p->priority) cur = cur->next;
        if (!cur) {
            // insert at tail
            p->prev = q->tail;
            p->next = NULL;
            q->tail->next = p;
            q->tail = p;
        } else {
            // insert before current
            p->next = cur;
            p->prev = cur->prev;
            if (cur->prev) cur->prev->next = p; else q->head = p;
            cur->prev = p;
        }
    }
    q->count++;
}

/**
 * @brief Enqueues a PCB into the blocked queue in FIFO order.
 * If the queue is empty, the PCB becomes both head and tail.
 * @param p Pointer to the PCB to be enqueued.
 */
void enqueue_blocked(PCB *p) {
    Queue *q = &g_blocked_queue;
    p->next = NULL;
    p->prev = q->tail;
    if (q->tail) q->tail->next = p;
    else q->head = p;
    q->tail = p;
    q->count++;
}

/**
 * @brief Enqueues a PCB into the suspended ready queue in descending priority order.
 * @details If the queue is empty, the PCB becomes both head and tail.
 * Otherwise, it finds the correct position based on priority.
 * @param p Pointer to the PCB to be enqueued.
 */
void enqueue_suspended_ready(PCB *p) {
    // insert by descending priority
    Queue *q = &g_suspended_ready_queue;
    if (!q->head) {
        q->head = q->tail = p;
        p->next = p->prev = NULL;
    } else {
        PCB *cur = q->head;
        while (cur && cur->priority >= p->priority) cur = cur->next;
        if (!cur) {
            // insert at tail
            p->prev = q->tail;
            p->next = NULL;
            q->tail->next = p;
            q->tail = p;
        } else {
            // insert before current
            p->next = cur;
            p->prev = cur->prev;
            if (cur->prev) cur->prev->next = p; else q->head = p;
            cur->prev = p;
        }
    }
    q->count++;
}

/**
 * @brief Enqueues a PCB into the suspended blocked queue in FIFO order.
 * @details If the queue is empty, the PCB becomes both head and tail.
 * @param p Pointer to the PCB to be enqueued.
 */
void enqueue_suspended_blocked(PCB *p) {
    Queue *q = &g_suspended_blocked_queue;
    p->next = NULL;
    p->prev = q->tail;
    if (q->tail) q->tail->next = p;
    else q->head = p;
    q->tail = p;
    q->count++;
}

/**
 * @brief Dequeues a specific PCB from the given queue.
 * If the PCB is found, it removes it from the queue and updates the head/tail pointers.
 * @param q Pointer to the queue from which to dequeue.
 * @param p Pointer to the PCB to be removed.
 */
void dequeue(Queue *q, PCB *p) {
    if (!p || !q->head) return;
    if (p->prev) p->prev->next = p->next; else q->head = p->next;
    if (p->next) p->next->prev = p->prev; else q->tail = p->prev;
    p->next = p->prev = NULL;
    q->count--;
}

void dump_queue(const char *title, Queue *q) {
    printf("================================ %s (%d) ==============================\n", title, q->count);
    int count = 0;
    for (PCB *p = q->head; p; p = p->next) {
        printf("------------------ Process %d ------------------\n", ++count);
        printf("Name: %s\n", p->p_name);
        printf("Process Class: %d\n", p->p_class);
        printf("State: %s\n", p->state==READY? "READY": p->state == RUNNING? "RUNNING" : "BLOCKED");
        printf("Suspended: %s\n", p->suspended? "true" : "false");
        printf("Priority: %d\n", p->priority);
        printf("-----------------------------------------------\n");
    }
}


/**
 * @brief Frees all PCBs in a queue and resets the queue.
 * @param q Pointer to the queue to be cleared.
 */
void cleanup_queue(Queue *q) {
    PCB *current = q->head;
    while (current != NULL) {
        PCB *next = current->next;
        free_pcb(current);
        current = next;
    }
    q->head = NULL;
    q->tail = NULL;
    q->count = 0;
}

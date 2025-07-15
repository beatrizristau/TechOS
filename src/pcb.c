#include "pcb.h"
#include "queue.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief Allocates memory for a new PCB (Process Control Block).
 * @return Pointer to the newly allocated PCB, or NULL if allocation fails.
 */
PCB *allocate_pcb(void) {
    PCB *newPCB = malloc(sizeof(PCB));

    if (newPCB == NULL) {
        return NULL;
    }
    memset(newPCB, 0, sizeof(PCB)); // Initialize the PCB to zero

    return newPCB;
}

/**
 * @brief Frees the memory allocated for a PCB.
 * @param p Pointer to the PCB to be freed.
 * @return 0 on success, -1 if the pointer is NULL.
 */
int free_pcb(PCB *p) {
    if (!p) return -1;
    free(p);
    return 0;
}

/**
 * @brief Inserts a PCB into the appropriate queue based on its state.
 * If the PCB is READY and not suspended, it is added to the ready queue.
 * Otherwise, it is added to the blocked queue.
 * @param p Pointer to the PCB to be inserted.
 */
void insert_pcb(PCB *p) {
    if (p->state == READY && !p->suspended) {
        enqueue_ready(p);
    } else if (p->state == BLOCKED && !p->suspended) {
        enqueue_blocked(p);
    } else if (p-> state == READY && p->suspended) {
        enqueue_suspended_ready(p);
    } else if (p->state == BLOCKED && p-> suspended) {
        enqueue_suspended_blocked(p);
    }
}

/**
 * @brief Sets up a PCB with the given parameters.
 * @param p_name Name of the process (up to 8 characters + '\0').
 * @param p_class Process class (0 for system, 1 for application).
 * @param priority Priority of the process (0-9).
 * @return Pointer to the newly created PCB, or NULL if allocation fails.
 */
PCB *setup_pcb(const char *p_name, const int p_class, const int priority, const char *file_path) {
    PCB *p = allocate_pcb();
    if (!p) return NULL;

    strncpy(p->p_name, p_name, 8);
    p->p_name[8] = '\0';
    p->p_class = p_class;
    p->priority = priority;
    p->state = READY;
    p->suspended = false;
    strncpy(p->file_path, file_path, PATH_MAX-1);
    p->offset = 0;

    insert_pcb(p);
    return p;
}

/**
 * @brief Finds a PCB by its name in the ready and blocked queues.
 * @param p_name Name of the process to search for.
 * @return Pointer to the PCB if found, NULL otherwise.
 */
PCB *find_pcb(const char *p_name) {
    PCB *p;

    /* search ready queue */
    for (p = g_ready_queue.head; p; p = p->next)
        if (strcmp(p->p_name, p_name) == 0) return p;

    /* search blocked queue */
    for (p = g_blocked_queue.head; p; p = p->next)
        if (strcmp(p->p_name, p_name) == 0) return p;

    /* search suspended ready queue */
    for (p = g_suspended_ready_queue.head; p; p = p->next)
        if (strcmp(p->p_name, p_name) == 0) return p;

    /* search suspended blocked queue */
    for (p = g_suspended_blocked_queue.head; p; p = p->next)
        if (strcmp(p->p_name, p_name) == 0) return p;

    return NULL;
}

/**
 * @brief Removes a PCB from its current queue (ready or blocked).
 * @param p Pointer to the PCB to be removed.
 * @return 0 on success, -1 if the pointer is NULL.
 */
int remove_pcb(PCB *p) {
    if (!p) return -1;

    if (p->state == READY && !p->suspended)
        dequeue(&g_ready_queue, p);
    else if (p->state == BLOCKED && !p->suspended)
        dequeue(&g_blocked_queue, p);
    else if (p->state == READY && p->suspended)
        dequeue(&g_suspended_ready_queue, p);
    else if (p->state == BLOCKED && p->suspended)
        dequeue(&g_suspended_blocked_queue, p);

    return 0;
}


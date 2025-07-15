#ifndef PCB_H
#define PCB_H

#include <limits.h>

#include <stdbool.h>

typedef enum { READY, RUNNING, BLOCKED } PCBState;

/* Process Control Block */
typedef struct pcb {
    char p_name[9]; // 8 chars + '\0'
    int p_class; // 0=system, 1=application
    int priority; // 0–9
    PCBState state; // ready, running, blocked
    bool suspended;
    struct pcb *next; // next PCB in the queue
    struct pcb *prev; // previous PCB in the queue
    char file_path[PATH_MAX]; // file that will be executed
    int offset; // offset in the file to start execution. 0 by default
} PCB;


PCB *allocate_pcb(void);
int free_pcb(PCB *p);
void insert_pcb(PCB *p);
PCB *setup_pcb(const char *p_name, int p_class, int priority, const char *file_path);
PCB *find_pcb(const char *p_name);
int remove_pcb(PCB *p);

#endif

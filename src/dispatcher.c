#include "dispatcher.h"
#include "queue.h"
#include "pcb.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "color_library.h"

void dispatch_all(void) {

    /* 1. Check if there are any processes in any queue before starting. */
    if (!g_ready_queue.head && !g_blocked_queue.head && !g_suspended_ready_queue.head && !g_suspended_blocked_queue.head) {
        printf("%sError: No processes to dispatch.%s\n", RED, RESET);
        return;
    }

    /* 2. Seed the random number generator for probabilistic unblocking. */
    srand(time(NULL));

    /* 3. Loop until all four queues are empty. */
    while (g_ready_queue.head || g_blocked_queue.head || g_suspended_ready_queue.head || g_suspended_blocked_queue.head) {
        PCB *p_to_unblock = NULL;

        /* --- UNBLOCKING PHASE --- */

        /* Find a candidate to unblock from any non-ready queue.
         * Priority: Blocked > Suspended Blocked > Suspended Ready */
        if (g_blocked_queue.head) {
            p_to_unblock = g_blocked_queue.head;
        } else if (g_suspended_blocked_queue.head) {
            p_to_unblock = g_suspended_blocked_queue.head;
        } else if (g_suspended_ready_queue.head) {
            p_to_unblock = g_suspended_ready_queue.head;
        }

        /* If a candidate for unblocking exists, decide whether to act. */
        if (p_to_unblock) {
            const bool must_unblock = !g_ready_queue.head;
            const bool maybe_unblock = (g_ready_queue.head && (rand() % 2 == 0));

            if (must_unblock || maybe_unblock) {
                printf("%sDispatcher: %s unblocking '%s'.%s\n", CYAN, must_unblock ? "Stall prevention," : "Probabilistically", p_to_unblock->p_name, RESET);
                remove_pcb(p_to_unblock);
                p_to_unblock->state = READY;
                p_to_unblock->suspended = false; // Always resume when unblocking
                insert_pcb(p_to_unblock);

                // If we had to unblock to prevent a stall, restart the loop
                // to ensure the newly ready process is dispatched next.
                if (must_unblock) {
                    continue;
                }
            }
        }

        /* --- DISPATCHING PHASE --- */

        /* If nothing is ready to run, continue to the next loop iteration.
         * This happens if probabilistic unblocking was possible but didn't happen. */
        if (!g_ready_queue.head) {
            continue;
        }

        PCB *p_to_run = g_ready_queue.head;
        remove_pcb(p_to_run);
        p_to_run->state = RUNNING;

        printf("%sDispatcher: Running '%s' (offset: %d)...%s\n", YELLOW, p_to_run->p_name, p_to_run->offset, RESET);

        char cmd[PATH_MAX + 32];
        snprintf(cmd, sizeof(cmd), "./execute %s %d", p_to_run->file_path, p_to_run->offset + 1);
        const int ret = system(cmd);

        if (ret == 0) {
            printf("%sDispatcher: Process '%s' completed successfully.%s\n", GREEN, p_to_run->p_name, RESET);
            free_pcb(p_to_run);
        } else {
            p_to_run->offset = ret / 256;
            p_to_run->state = BLOCKED;
            p_to_run->suspended = true;
            insert_pcb(p_to_run); // This will place it in the suspended-blocked queue
            printf("%sDispatcher: Process '%s' interrupted. New offset: %d.%s\n", MAGENTA, p_to_run->p_name, p_to_run->offset, RESET);
        }
    }

    printf("%sDispatcher: All processes have finished execution.%s\n", GREEN, RESET);
}

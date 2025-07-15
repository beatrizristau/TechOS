#include <stdio.h>

#include "color_library.h"
#include "comhan.h"
#include "utils.h"
#include "queue.h"
#include "auth.h"

/**
 * @brief Displays the welcome message for TechOS.
*/
void welcome_message(void) {
    printf("Welcome to TechOS!\n");
}

/**
 * @brief Initializes TechOS resources.
*/
void init_techos(void) {
    init_techos_date();
    init_queues();

    /* Add other initializations here if needed in the future */
    printf("%sTechOS Initialized.%s\n", MAGENTA, RESET);
}

/**
 * @brief Performs cleanup before TechOS exits.
*/
void cleanup_techos(void) {
    // Add cleanup tasks here (e.g., freeing allocated memory) if needed
    printf("%sPerforming TechOS cleanup...%s\n", MAGENTA, RESET);
    cleanup_queue(&g_ready_queue);
    cleanup_queue(&g_blocked_queue);
    cleanup_queue(&g_suspended_ready_queue);
    cleanup_queue(&g_suspended_blocked_queue);
    printf("%sTechOS cleanup completed.%s\n", MAGENTA, RESET);
}

/**
 * @brief Displays the exit message for TechOS.
*/
void exit_message(void) {
    printf("%sShutting down TechOS. Goodbye!%s\n", GREEN, RESET);
}

/**
 * @brief Main entry point for TechOS.
 * @return 0 on successful execution.
*/
int main(void) {
    welcome_message();

    if (!handle_login()) {
        exit_message();
        return 0;
    }

    init_techos();
    comhan();
    cleanup_techos();
    exit_message();

    return 0;
}

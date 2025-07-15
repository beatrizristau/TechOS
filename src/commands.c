#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

#include "color_library.h"
#include "commands.h"
#include "utils.h"
#include "comhan.h"
#include "dispatcher.h"
#include "pcb.h"
#include "queue.h"
#include "auth.h"


/**
 * @brief An array of error messages corresponding to DateValidationResult enums.
*/
static const char* const g_date_error_messages[] = {
    [DATE_ERROR_INVALID_MONTH] = "Invalid month. Please enter a value between 1 and 12.",
    [DATE_ERROR_INVALID_YEAR]  = "Invalid year. Please enter a year after 1900.",
    [DATE_ERROR_INVALID_DAY]   = "Invalid day for the given month and year."
};

/**
 * @brief The 'help' command displays help information related to TechOS available commands.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_help(const int argc, char *argv[]) {
    char *topic = "summary";
    if (argc > 1) {
        topic = argv[1];
    }

    char path[512];
    snprintf(path, sizeof(path), "%s/%s.txt", HELP_DIR, topic);
    print_file(path);
    printf("\n");
}

/**
 * @brief The 'version' command displays information about the TechOS version, authors, and completion date.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_version(const int argc, char *argv[]) {
    (void)argc; (void)argv; // unused parameters
    printf("----------------------------------- Version ----------------------------------\n");
    printf("Version: %s\n", TECHOS_VERSION);
    printf("Authors: %s\n", AUTHORS);
    printf("Completion Date: %s\n", COMPLETION_DATE);
    printf("------------------------------------------------------------------------------\n");
}

/**
 * @brief The 'showdate' command displays the current TechOS date.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_show_date(const int argc, char *argv[]) {
    (void)argc; (void)argv; // unused parameters
    char date_buffer[20]; // buffer for MM-DD-YYYY + null terminator
    get_techos_date_str(date_buffer, sizeof(date_buffer));

    printf("-------------------------------- TechOS Date ---------------------------------\n");
    printf("Current TechOS Date: %s", date_buffer);
    printf("\n------------------------------------------------------------------------------\n");
}

/**
 * @brief The 'setdate' command allows the user to set the TechOS date.
 * @details It checks if the date format is valid (MM-DD-YYYY) and if the date values are logically valid (e.g., day in month, leap year).
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_set_date(const int argc, char *argv[]) {
    (void)argc; // unused parameter
    int month, day, year;

    /* First check if the format is valid (MM-DD-YYYY) */
    if (sscanf(argv[1], "%d-%d-%d", &month, &day, &year) != 3) {
        printf("%sError: Invalid date format. Please use MM-DD-YYYY.%s\n", RED, RESET);
        return;
    }

    const DateValidationResult validation_result = validate_date_components(month, day, year);

    if (validation_result == DATE_VALID) {
        set_techos_date(month, day, year);
        char date_buffer[20];
        get_techos_date_str(date_buffer, sizeof(date_buffer));
        printf("%sTechOS date successfully changed to: %s%s\n", GREEN, date_buffer, RESET);
    } else {
        printf("%sError: %s%s\n", RED, g_date_error_messages[validation_result], RESET);
    }
}

/**
 * @brief The 'showtime' command displays the current TechOS time.
 * @details displays the current system time in 24-hour format.
 * @param argc Argument count.
 * @param argv Argument vector.
*/
void handle_show_time(const int argc, char *argv[]) {
    (void)argc; (void)argv; // unused parameters
    char time_buffer[20];
    get_current_time_str(time_buffer, sizeof(time_buffer));
    printf("-------------------------------- TechOS Time ---------------------------------\n");
    printf("Current System Time: %s", time_buffer);
    printf("\n------------------------------------------------------------------------------\n");
}

/**
 * @brief The 'exit' or 'quit' command terminates the TechOS session.
 * @details it prompts the user for confirmation before exiting.
 * @param argc Argument count.
 * @param argv Argument vector.
*/
void handle_terminate(const int argc, char *argv[]) {
    (void)argc; (void)argv; // unused parameters
    char confirmation[10]; // Buffer for user confirmation input

    printf("%sAre you sure you want to exit TechOS? (yes/no):%s ", BLUE, RESET);
    if (fgets(confirmation, sizeof(confirmation), stdin)) {
        /* Remove trailing newline if present */
        confirmation[strcspn(confirmation, "\n")] = 0;

        if (strcmp(confirmation, "yes") == 0) {
            g_comhan_running = 0;
            return;
        }
    }
    printf("%sTermination cancelled.%s\n", MAGENTA, RESET);
}

/**
 * @brief The 'createpcb' command creates a new Process Control Block (PCB).
 * @details Function will call setup_pcb and insert the PCB in the appropriate queue.
 * The command takes the process name, class, and priority as parameters.
 * The command should check that the name is unique and valid, the class is valid, and the priority is valid.
 * Otherwise, appropriate error messages should be given.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_create_pcb(const int argc, char *argv[]) {
    (void)argc; // unused parameter

    /* validate name */
    char const *p_name = argv[1];
    if (!validate_name(p_name)) {
        printf("%sError: Invalid name. Process Name must be between 1 and 8 characters. First character must be a letter.%s\n", RED, RESET);
        return;
    }

    /* validate class */
    int p_class;
    if (!validate_class(argv[2], &p_class)) {
        printf("%sError: Class must be an integer 0 (system) or 1 (application).%s\n", RED, RESET);
        return;
    }

    /* validate priority */
    int priority;
    if (!validate_priority(argv[3], &priority)) {
        printf("%sError: Priority must be an integer between 0 and 9.%s\n", RED, RESET);
        return;
    }

    if (find_pcb(p_name)) {
        printf("%sError: Name already in use.%s\n", RED, RESET); return;
    }

    PCB const *p = setup_pcb(p_name, p_class, priority, "");
    if (!p) {
        printf("%sError: Could not allocate PCB.%s\n", RED, RESET); return;
    }
    printf("%sPCB '%s' created (class=%u, priority=%d).%s\n", GREEN, p_name, p_class, priority, RESET);
}

/**
 * @brief The 'showallpcbs' command displays all currently active PCBs.
 * @details It retrieves and prints all PCBs in the system.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_show_all_pcbs(const int argc, char *argv[]) {
    (void)argc; (void)argv; // unused parameters
    dump_queue("Ready Queue", &g_ready_queue);
    dump_queue("Blocked Queue", &g_blocked_queue);
    dump_queue("Suspended Ready Queue", &g_suspended_ready_queue);
    dump_queue("Suspended Blocked Queue", &g_suspended_blocked_queue);
}

/**
 * @brief The 'deletepcb' command deletes a PCB by its name.
 * @details It searches for the PCB in both the ready and blocked queues and removes it if found.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_delete_pcb(const int argc, char *argv[]) {
    (void)argc; // unused parameter
    char const *p_name = argv[1];

    PCB *p = find_pcb(p_name);
    if (!p) {
        printf("%sError: PCB '%s' not found.%s\n", RED, p_name, RESET);
        return;
    }

    if (remove_pcb(p) == 0) {
        printf("%sPCB '%s' deleted successfully.%s\n", GREEN, p_name, RESET);
    } else {
        printf("%sError: Could not delete PCB '%s'.%s\n", RED, p_name, RESET);
    }
}

/**
 * @brief The 'blockpcb' command blocks a PCB by its name.
 * @details It searches for the PCB and moves it to the blocked queue if found.
 * It does not change the suspended status of the PCB.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_block_pcb(const int argc, char *argv[]) {
    (void)argc; // unused parameter
    char const *p_name = argv[1];

    PCB *p = find_pcb(p_name);
    if (!p) {
        printf("%sError: PCB '%s' not found.%s\n", RED, p_name, RESET);
        return;
    }

    if (p->state == BLOCKED) {
        printf("%sPCB '%s' is already blocked.%s\n", YELLOW, p_name, RESET);
        return;
    }

    if (remove_pcb(p) == -1) {
        printf("%sError: Could not remove PCB '%s' from its current queue.%s\n", RED, p_name, RESET);
        return;
    }

    p->state = BLOCKED;
    insert_pcb(p);
    printf("%sPCB '%s' blocked successfully.%s\n", GREEN, p_name, RESET);
}

/**
 * @brief The 'unblockpcb' command unblocks a PCB by its name.
 * @details It searches for the PCB in the blocked queue and moves it back to the ready queue if found.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_unblock_pcb(const int argc, char *argv[]) {
    (void)argc; // unused parameter
    char const *p_name = argv[1];

    PCB *p = find_pcb(p_name);
    if (!p) {
        printf("%sError: PCB '%s' not found.%s\n", RED, p_name, RESET);
        return;
    }

    if (p->state != BLOCKED) {
        printf("%sPCB '%s' is not blocked.%s\n", YELLOW, p_name, RESET);
        return;
    }
    if (remove_pcb(p) == -1) {
        printf("%sError: Could not remove PCB '%s' from the blocked queue.%s\n", RED, p_name, RESET);
        return;
    }
    p->state = READY;
    insert_pcb(p);
    printf("%sPCB '%s' unblocked successfully.%s\n", GREEN, p_name, RESET);
}

/**
 * @brief The 'suspendpcb' command suspends a PCB by its name.
 * @details It moves the PCB to the suspended ready queue if it is in the ready queue,
 * or to the suspended blocked queue if it is in the blocked queue.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_suspend_pcb(const int argc, char *argv[]) {
    (void)argc; // unused parameter
    char const *p_name = argv[1];

    PCB *p = find_pcb(p_name);
    if (!p) {
        printf("%sError: PCB '%s' not found.%s\n", RED, p_name, RESET);
        return;
    }

    if (p->suspended) {
        printf("%sPCB '%s' is already suspended.%s\n", YELLOW, p_name, RESET);
        return;
    }

    remove_pcb(p);
    p->suspended = true;
    insert_pcb(p);

    printf("%sPCB '%s' suspended successfully.%s\n", GREEN, p_name, RESET);
}

/**
 * @brief The 'resumepcb' command places a PCB in the not suspended state and reinserts it into the appropriate queue.
 * @details It moves the PCB back to the ready queue if it was suspended or to the blocked queue if it was suspended while blocked.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_resume_pcb(const int argc, char *argv[]) {
    (void)argc; // unused parameter
    char const *p_name = argv[1];

    PCB *p = find_pcb(p_name);
    if (!p) {
        printf("%sError: PCB '%s' not found.%s\n", RED, p_name, RESET);
        return;
    }

    if (!p->suspended) {
        printf("%sPCB '%s' is already not suspended.%s\n", YELLOW, p_name, RESET);
        return;
    }

    remove_pcb(p);
    p->suspended = false;
    insert_pcb(p);

    printf("%sPCB '%s' resumed successfully.%s\n", GREEN, p_name, RESET);
}

/**
 * @brief The 'setpcbpriority' command sets the priority of a PCB by its name.
 * @details It searches for the PCB and updates its priority and re-inserts it into the appropriate queue.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_set_pcb_priority(const int argc, char *argv[]) {
    (void)argc; // unused parameter
    char const *p_name = argv[1];

    PCB *p = find_pcb(p_name);
    if (!p) {
        printf("%sError: PCB '%s' not found.%s\n", RED, p_name, RESET);
        return;
    }

    /* validate priority */
    int priority;
    if (!validate_priority(argv[2], &priority)) {
        printf("%sError: Priority must be an integer between 0 and 9.%s\n", RED, RESET);
        return;
    }

    remove_pcb(p);
    p->priority = priority;
    insert_pcb(p);

    printf("%sPCB '%s' priority set to %d successfully.%s\n", GREEN, p_name, priority, RESET);
}

/**
 * @brief The 'showpcb' command displays the details of a PCB by its name.
 * @details It retrieves and prints the PCB's information if found.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_show_pcb(const int argc, char *argv[]) {
    (void)argc; // unused parameter
    char const *p_name = argv[1];

    PCB *p = find_pcb(p_name);
    if (!p) { printf("%sError: PCB '%s' not found.%s\n", RED, p_name, RESET); return; }

    printf("------------------ Process %s ------------------\n", p_name);
    printf("Name: %s\n", p->p_name);
    printf("Process Class: %d\n", p->p_class);
    printf("State: %s\n", p->state==READY? "READY": p->state == RUNNING? "RUNNING" : "BLOCKED");
    printf("Suspended: %s\n", p->suspended? "true" : "false");
    printf("Priority: %d\n", p->priority);
    printf("-----------------------------------------------\n");
}

/**
 * @brief The 'showreadypcbs' command displays all PCBs in the ready queue.
 * @details It retrieves and prints all PCBs that are currently in the ready state.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_show_ready_pcbs(const int argc, char *argv[]) {
    (void)argc; (void)argv; // unused parameters

    dump_queue("Ready Queue", &g_ready_queue);
    dump_queue("Ready Suspended Queue", &g_suspended_ready_queue);
}

/**
 * @brief The 'showblockedpcbs' command displays all PCBs in the blocked queue.
 * @details It retrieves and prints all PCBs that are currently in the blocked state.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_show_blocked_pcbs(const int argc, char *argv[]) {
    (void)argc; (void)argv; // unused parameters

    dump_queue("Blocked Queue", &g_blocked_queue);
    dump_queue("Blocked Suspended Queue", &g_suspended_blocked_queue);
}

/**
 * @brief The 'loadpcbs' command loads a PCB from a file and adds it to the system.
 * @details It reads the PCB information from a file and creates a new PCB with the specified parameters.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_load_pcbs(const int argc, char *argv[]) {
    (void)argc; // unused parameter
    const int p_class = 1;

    /* validate name */
    char const *p_name = argv[1];
    if (!validate_name(p_name)) {
        printf("%sError: Invalid name. Process Name must be between 1 and 8 characters. First character must be a letter.%s\n", RED, RESET);
        return;
    }

    /* validate priority */
    int priority;
    if (!validate_priority(argv[2], &priority)) {
        printf("%sError: Priority must be an integer between 0 and 9.%s\n", RED, RESET);
        return;
    }

    /* validate file path */
    char file_path_buffer[256];
    snprintf(file_path_buffer, sizeof(file_path_buffer), "%s.techos", argv[3]);
    char const *file_path = file_path_buffer;
    if (!validate_file_path(file_path)) {
        return;
    }

    if (find_pcb(p_name)) {
        printf("%sError: Name already in use.%s\n", RED, RESET); return;
    }

    PCB const *p = setup_pcb(p_name, p_class, priority, file_path);
    if (!p) {
        printf("%sError: Could not allocate PCB.%s\n", RED, RESET); return;
    }
    printf("%sPCB '%s' created (class=%u, priority=%d, file_path=%s).%s\n", GREEN, p_name, p_class, priority, file_path, RESET);
}

/**
 * @brief The 'dispatchpcbs' command dispatches all PCBs in the ready queue.
 * @details It processes each PCB in the ready queue, executing them and handling their states.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_dispatch_pcbs(const int argc, char *argv[]) {
    (void)argc; (void)argv; // unused parameters
    dispatch_all();
}

/**
 * @brief The 'clear' command clears the terminal screen.
 * @details It uses ANSI escape codes to clear the screen and move the cursor to the top.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_clear(const int argc, char *argv[]) {
    (void)argc; (void)argv; // unused parameters
    printf("\033[2J\033[H"); //  clear screen and move cursor to top
}

/**
 * @brief The 'ls' command lists the contents of a directory.
 * @details It lists files and folders in the given directory. Supports an '-l' option to show file sizes.
 * If no path is provided, it lists the contents of the current directory.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_view_directory(const int argc, char *argv[]) {
    bool show_size = false;
    char path_buffer[512] = {0};
    char *path = ".";
    int path_parts = 0;

    /* Parse arguments to identify the '-l' flag and construct the directory path.
     * This allows the path to be specified with spaces, even without quotes,
     * by concatenating all non-flag arguments.
    */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            show_size = true;
        } else {
            if (path_parts > 0) {
                strcat(path_buffer, " ");
            }
            strcat(path_buffer, argv[i]);
            path_parts++;
        }
    }

    /* If a path was constructed from the arguments, use it. */
    if (path_parts > 0) {
        path = path_buffer;
    }

    /* Attempt to open the specified directory. */
    DIR *d = opendir(path);
    if (d == NULL) {
        printf("%sError: Cannot open directory '%s': %s%s\n", RED, path, strerror(errno), RESET);
        return;
    }

    /* Iterate through each entry in the directory. */
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        /* Skip hidden files (those starting with a '.'). */
        if (dir->d_name[0] == '.') {
            continue;
        }

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, dir->d_name);

        /* Retrieve metadata for the directory entry. */
        struct stat statbuf;
        if (stat(full_path, &statbuf) == -1) {
            /* If stat fails, just print the name without extra info. */
            printf("%s%s%s\n", GREY, dir->d_name, RESET);
            continue;
        }

        const bool is_dir = S_ISDIR(statbuf.st_mode);

        /* Print directories in a different color for better visibility. */
        if (is_dir) {
            printf("%s%s/%s", CYAN, dir->d_name, RESET);
        } else {
            printf("%s%s%s", GREY, dir->d_name, RESET);
        }

        /* If the '-l' option is enabled and the entry is a file, display its size. */
        if (show_size && !is_dir) {
            printf(" %10ld bytes", statbuf.st_size);
        }
        printf("\n");
    }

    closedir(d);
}

/**
 * @brief The 'cd' command changes the current working directory.
 * @details Changes the directory to the path specified by the user. If no path is given,
 * it changes to the HOME directory. Supports absolute and relative paths.
 * Reports an error if the path is invalid.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_change_directory(const int argc, char *argv[]) {
    char *path;

    if (argc == 1) {
        path = getenv("HOME");
        if (path == NULL) {
            printf("%sError: HOME environment variable not set.%s\n", RED, RESET);
            return;
        }
    } else {
        path = argv[1];
    }

    if (chdir(path) != 0) {
        printf("%sError: Cannot change directory to '%s': %s%s\n", RED, path, strerror(errno), RESET);
    } else {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%sCurrent directory is now: %s%s\n", GREEN, cwd, RESET);
        } else {
            // If getcwd fails, we are in a valid but unreadable directory.
            // Report success with the requested path for clarity.
            printf("%sCurrent directory changed to %s%s\n", GREEN, path, RESET);
        }
    }
}

/**
 * @brief The 'mkdir' command creates a new directory.
 * @details Creates a new directory with the specified name in the current location.
 * The folder name can contain spaces if enclosed in quotes.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_create_directory(const int argc, char *argv[]) {
    (void)argc; // unused parameter
    char *folder_name = argv[1];

    /* Attempt to create the directory with default permissions (read, write, execute for all). */
    if (mkdir(folder_name, 0777) != 0) {
        // If mkdir fails, report the specific reason.
        if (errno == EEXIST) {
            printf("%sError: Folder '%s' already exists.%s\n", RED, folder_name, RESET);
        } else {
            printf("%sError: Could not create folder '%s': %s%s\n", RED, folder_name, strerror(errno), RESET);
        }
    } else {
        printf("%sFolder '%s' created successfully.%s\n", GREEN, folder_name, RESET);
    }
}

/**
 * @brief The 'rmdir' command removes an empty directory.
 * @details Deletes the specified folder if it is empty. Prompts for user confirmation first.
 * The folder name can contain spaces if enclosed in quotes.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_remove_directory(const int argc, char *argv[]) {
    (void)argc;
    char *folder_name = argv[1];

    struct stat statbuf;
    if (stat(folder_name, &statbuf) != 0) {
        printf("%sError: Folder '%s' not found: %s%s\n", RED, folder_name, strerror(errno), RESET);
        return;
    }

    if (!S_ISDIR(statbuf.st_mode)) {
        printf("%sError: '%s' is not a folder.%s\n", RED, folder_name, RESET);
        return;
    }

    if (is_directory_empty(folder_name)) {
        printf("%sError: Folder '%s' is not empty.%s\n", RED, folder_name, RESET);
        return;
    }

    char confirmation[10];
    printf("%sAre you sure you want to delete the folder '%s'? (yes/no):%s ", BLUE, folder_name, RESET);
    if (fgets(confirmation, sizeof(confirmation), stdin)) {
        confirmation[strcspn(confirmation, "\n")] = 0; // Remove newline

        if (strcmp(confirmation, "yes") == 0) {
            if (rmdir(folder_name) == 0) {
                printf("%sFolder '%s' removed successfully.%s\n", GREEN, folder_name, RESET);
            } else {
                printf("%sError: Could not remove folder '%s': %s%s\n", RED, folder_name, strerror(errno), RESET);
            }
        } else {
            printf("%sRemoval of folder '%s' cancelled.%s\n", MAGENTA, folder_name, RESET);
        }
    } else {
        printf("\n%sInvalid input. Removal cancelled.%s\n", RED, RESET);
    }
}

/**
 * @brief The 'touch' command creates a new, empty file.
 * @details Creates a file with the specified name. If a file with the same name
 * already exists, it reports an error. The file name can contain spaces if
 * enclosed in quotes.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_create_file(const int argc, char *argv[]) {
    (void)argc;
    char *file_name = argv[1];

    // O_CREAT: Create the file if it does not exist.
    // O_EXCL: When used with O_CREAT, fail if the file already exists.
    // This provides an atomic check-and-create operation.
    // 0666 provides read/write permissions for user, group, and others.
    const int fd = open(file_name, O_CREAT | O_EXCL, 0666);

    if (fd == -1) {
        if (errno == EEXIST) {
            printf("%sError: File '%s' already exists.%s\n", RED, file_name, RESET);
        } else {
            printf("%sError: Could not create file '%s': %s%s\n", RED, file_name, strerror(errno), RESET);
        }
    } else {
        close(fd); // Immediately close the file descriptor.
        printf("%sFile '%s' created successfully.%s\n", GREEN, file_name, RESET);
    }
}

/**
 * @brief The 'rm' command removes a file.
 * @details Deletes the specified file after prompting the user for confirmation.
 * The file name can contain spaces if enclosed in quotes.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_remove_file(const int argc, char *argv[]) {
    (void)argc;
    char *file_name = argv[1];

    struct stat statbuf;
    // First, check if the entry exists.
    if (stat(file_name, &statbuf) != 0) {
        printf("%sError: File '%s' not found: %s%s\n", RED, file_name, strerror(errno), RESET);
        return;
    }

    // Ensure that it's a regular file and not a directory.
    if (!S_ISREG(statbuf.st_mode)) {
        printf("%sError: '%s' is not a regular file. Use 'rmdir' for empty directories.%s\n", RED, file_name, RESET);
        return;
    }

    char confirmation[10];
    printf("%sAre you sure you want to delete the file '%s'? (yes/no):%s ", BLUE, file_name, RESET);
    if (fgets(confirmation, sizeof(confirmation), stdin)) {
        confirmation[strcspn(confirmation, "\n")] = 0; // Remove newline

        if (strcmp(confirmation, "yes") == 0) {
            if (remove(file_name) == 0) {
                printf("%sFile '%s' removed successfully.%s\n", GREEN, file_name, RESET);
            } else {
                printf("%sError: Could not remove file '%s': %s%s\n", RED, file_name, strerror(errno), RESET);
            }
        } else {
            printf("%sRemoval of file '%s' cancelled.%s\n", MAGENTA, file_name, RESET);
        }
    } else {
        printf("\n%sInvalid input. Removal cancelled.%s\n", RED, RESET);
    }
}

/**
 * @brief The 'adduser' command creates a new user account.
 * @details Only admins and root can create users. Admins can only create basic users.
 * The command checks for pre-existing users before creation.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_add_user(const int argc, char *argv[]) {
    (void)argc; // unused parameter
    const User *current_user = get_current_user();

    /* 1. Check permissions: only admins or root can proceed. */
    if (current_user->role < ROLE_ADMIN) {
        printf("%sError: Permission denied. Must be an admin or root.%s\n", RED, RESET);
        return;
    }

    char *username = argv[1];
    char *password = argv[2];
    char *role_str = argv[3];

    /* 2. Validate the role parameter. */
    if (strcmp(role_str, "basic") != 0 && strcmp(role_str, "admin") != 0) {
        printf("%sError: Invalid role. Must be 'basic' or 'admin'.%s\n", RED, RESET);
        return;
    }

    /* 3. Enforce role hierarchy: Admins cannot create other admins. */
    if (current_user->role == ROLE_ADMIN && strcmp(role_str, "admin") == 0) {
        printf("%sError: Admins can only create 'basic' users.%s\n", RED, RESET);
        return;
    }

    FILE *file = fopen(ACCOUNTS_FILE, "a+");
    if (file == NULL) {
        printf("%sError: Could not open accounts file.%s\n", RED, RESET);
        return;
    }

    /* 4. Check if the user already exists to prevent duplicates. */
    char line[512];
    bool exists = false;
    rewind(file); // Start reading from the beginning of the file.
    while (fgets(line, sizeof(line), file)) {
        char line_copy[512];
        strncpy(line_copy, line, sizeof(line_copy));
        line_copy[sizeof(line_copy) - 1] = '\0'; // Ensure null termination

        // Safely tokenize the copied line.
        const char *existing_user = strtok(line_copy, ",");
        if (existing_user && strcmp(existing_user, username) == 0) {
            exists = true;
            break;
        }
    }

    if (exists) {
        printf("%sError: User '%s' already exists.%s\n", RED, username, RESET);
    } else {
        // 5. Append the new user to the file with a preceding newline.
        fprintf(file, "\n%s,%s,%s", username, password, role_str);
        printf("%sUser '%s' created successfully.%s\n", GREEN, username, RESET);
    }
    fclose(file);
}

/**
 * @brief The 'removeuser' command deletes a user account.
 * @details Only admins and root can delete users. Admins can only delete basic users.
 * The command checks for the existence of the user before deletion.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_remove_user(const int argc, char *argv[]) {
    (void)argc; // unused parameter
    const User *current_user = get_current_user();
    char *user_to_delete = argv[1];

    /* 1. Basic permission check */
    if (current_user->role < ROLE_ADMIN) {
        printf("%sError: Permission denied. Must be an admin or root. %s\n", RED, RESET);
        return;
    }

    /* 2. Prevent root from being deleted */
    if (strcmp(user_to_delete, "root") == 0) {
        printf("%sError: The root administrator cannot be deleted.%s\n", RED, RESET);
        return;
    }

    /* 3. Prevent self-deletion */
    if (strcmp(user_to_delete, current_user->username) == 0) {
        printf("%sError: You cannot delete your own account.%s\n", RED, RESET);
        return;
    }

    /* --- Read all accounts into memory --- */
    FILE *file = fopen("accounts.txt", "r");
    if (file == NULL) {
        printf("%sError: Could not open accounts file.%s\n", RED, RESET);
        return;
    }

    typedef struct { char line[512]; char user[MAX_USERNAME_LEN]; UserRole role; } Account;
    Account *accounts = NULL;
    int count = 0;
    char buffer[512];

    while (fgets(buffer, sizeof(buffer), file)) {
        accounts = realloc(accounts, (count + 1) * sizeof(Account));
        strncpy(accounts[count].line, buffer, sizeof(accounts[count].line));

        char temp_user[MAX_USERNAME_LEN];
        char temp_role[50];
        sscanf(buffer, "%[^,],%*[^,],%s", temp_user, temp_role);

        strncpy(accounts[count].user, temp_user, MAX_USERNAME_LEN);
        accounts[count].role = (strcmp(temp_role, "admin") == 0) ? ROLE_ADMIN : ROLE_BASIC;
        if(strcmp(temp_role, "root") == 0) accounts[count].role = ROLE_ROOT;

        count++;
    }
    fclose(file);

    /* --- Find user and check permissions --- */
    int target_index = -1;
    for (int i = 0; i < count; i++) {
        if (strcmp(accounts[i].user, user_to_delete) == 0) {
            target_index = i;
            break;
        }
    }

    if (target_index == -1) {
        printf("%sError: User '%s' not found.%s\n", RED, user_to_delete, RESET);
        free(accounts);
        return;
    }

    /* 4. Admins cannot delete other admins */
    if (current_user->role == ROLE_ADMIN && accounts[target_index].role == ROLE_ADMIN) {
        printf("%sError: Administrators cannot delete other administrators.%s\n", RED, RESET);
        free(accounts);
        return;
    }

    /* --- Rewrite the accounts file without the deleted user --- */
    file = fopen("accounts.txt", "w");
    if (file == NULL) {
        printf("%sError: Could not rewrite accounts file.%s\n", RED, RESET);
        free(accounts);
        return;
    }

    for (int i = 0; i < count; i++) {
        if (i != target_index) {
            fprintf(file, "%s", accounts[i].line);
        }
    }

    fclose(file);
    free(accounts);

    printf("%sUser '%s' has been removed successfully.%s\n", GREEN, user_to_delete, RESET);
}

/**
 * @brief The 'passwd' command changes a user's password.
 * @details Enforces role permissions for who can change whose password.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_change_password(const int argc, char *argv[]) {
    (void)argc; // unused parameter
    const User *current_user = get_current_user();
    char *target_username = argv[1];

    // --- Read all accounts into memory ---
    FILE *file = fopen("accounts.txt", "r");
    if (file == NULL) {
        printf("%sError: Could not open accounts file.%s\n", RED, RESET);
        return;
    }

    typedef struct { char user[MAX_USERNAME_LEN]; char pass[MAX_PASSWORD_LEN]; char role_str[20]; UserRole role; } Account;
    Account *accounts = NULL;
    int count = 0;
    char buffer[512];
    int target_index = -1;

    while (fgets(buffer, sizeof(buffer), file)) {
        buffer[strcspn(buffer, "\r\n")] = 0;
        if(strlen(buffer) == 0) continue;

        accounts = realloc(accounts, (count + 1) * sizeof(Account));
        sscanf(buffer, "%[^,],%[^,],%s", accounts[count].user, accounts[count].pass, accounts[count].role_str);

        if (strcmp(accounts[count].role_str, "root") == 0) accounts[count].role = ROLE_ROOT;
        else if (strcmp(accounts[count].role_str, "admin") == 0) accounts[count].role = ROLE_ADMIN;
        else accounts[count].role = ROLE_BASIC;

        if (strcmp(accounts[count].user, target_username) == 0) {
            target_index = count;
        }
        count++;
    }
    fclose(file);

    if (target_index == -1) {
        printf("%sError: User '%s' not found.%s\n", RED, target_username, RESET);
        free(accounts);
        return;
    }

    // --- Permission Checks ---
    const UserRole target_role = accounts[target_index].role;
    const bool is_self_change = strcmp(current_user->username, target_username) == 0;
    bool can_change = false;

    if (target_role == ROLE_ROOT) {
        // Rule: Only root can change the root password (must be a self-change).
        if (is_self_change && current_user->role == ROLE_ROOT) {
            can_change = true;
        }
    } else if (is_self_change) {
        // Rule: Any non-root user can change their own password.
        can_change = true;
    } else if (current_user->role == ROLE_ROOT) {
        // Rule: Root can change any other non-root user's password.
        can_change = true;
    } else if (current_user->role == ROLE_ADMIN && target_role == ROLE_BASIC) {
        // Rule: Admins can change basic users' passwords.
        can_change = true;
    }

    if (!can_change) {
        if (target_role == ROLE_ROOT) {
            printf("%sError: Permission denied. Only root can change the root password.%s\n", RED, RESET);
        } else if (current_user->role == ROLE_ADMIN && target_role == ROLE_ADMIN) {
            printf("%sError: Permission denied. Administrators cannot change another administrator's password.%s\n", RED, RESET);
        } else {
            printf("%sError: Permission denied to change password for '%s'.%s\n", RED, target_username, RESET);
        }
        free(accounts);
        return;
    }

    // --- Prompt for new password ---
    char new_pass1[MAX_PASSWORD_LEN], new_pass2[MAX_PASSWORD_LEN];
    printf("Enter new password for %s: ", target_username);
    fflush(stdout);
    get_secure_password(new_pass1, sizeof(new_pass1));
    printf("\nConfirm new password: ");
    fflush(stdout);
    get_secure_password(new_pass2, sizeof(new_pass2));

    if (strcmp(new_pass1, new_pass2) != 0) {
        printf("\n%sError: Passwords do not match.%s\n", RED, RESET);
        free(accounts);
        return;
    }

    // --- Update password and rewrite file ---
    strncpy(accounts[target_index].pass, new_pass1, MAX_PASSWORD_LEN - 1);
    accounts[target_index].pass[MAX_PASSWORD_LEN - 1] = '\0';

    file = fopen("accounts.txt", "w");
    if (file == NULL) {
        printf("%sError: Could not rewrite accounts file.%s\n", RED, RESET);
        free(accounts);
        return;
    }

    for (int i = 0; i < count; i++) {
        fprintf(file, "%s,%s,%s%s", accounts[i].user, accounts[i].pass, accounts[i].role_str, (i == count - 1) ? "" : "\n");
    }
    fclose(file);
    free(accounts);

    printf("\n%sPassword for user '%s' changed successfully.%s\n", GREEN, target_username, RESET);
}

/**
 * @brief The 'addadmin' command gives a user administrator privileges.
 * @details Only admins and root can grant admin privileges. The target user must exist and not already be an admin.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_add_admin(const int argc, char *argv[]) {
    (void)argc; // unused parameter
    const User *current_user = get_current_user();
    char *target_username = argv[1];

    // 1. Permission check: Only root or admins can grant admin privileges.
    if (current_user->role < ROLE_ADMIN) {
        printf("%sError: Permission denied. Must be an admin or root.%s\n", RED, RESET);
        return;
    }

    // --- Read all accounts into memory ---
    FILE *file = fopen("accounts.txt", "r");
    if (file == NULL) {
        printf("%sError: Could not open accounts file.%s\n", RED, RESET);
        return;
    }

    typedef struct { char user[MAX_USERNAME_LEN]; char pass[MAX_PASSWORD_LEN]; char role_str[20]; UserRole role; } Account;
    Account *accounts = NULL;
    int count = 0;
    char buffer[512];
    int target_index = -1;

    while (fgets(buffer, sizeof(buffer), file)) {
        buffer[strcspn(buffer, "\r\n")] = 0;
        if(strlen(buffer) == 0) continue;

        accounts = realloc(accounts, (count + 1) * sizeof(Account));
        sscanf(buffer, "%[^,],%[^,],%s", accounts[count].user, accounts[count].pass, accounts[count].role_str);

        if (strcmp(accounts[count].role_str, "root") == 0) accounts[count].role = ROLE_ROOT;
        else if (strcmp(accounts[count].role_str, "admin") == 0) accounts[count].role = ROLE_ADMIN;
        else accounts[count].role = ROLE_BASIC;

        if (strcmp(accounts[count].user, target_username) == 0) {
            target_index = count;
        }
        count++;
    }
    fclose(file);

    // 2. Check if user exists
    if (target_index == -1) {
        printf("%sError: User '%s' not found.%s\n", RED, target_username, RESET);
        free(accounts);
        return;
    }

    // 3. Check if user is already an admin or root
    if (accounts[target_index].role >= ROLE_ADMIN) {
        printf("%sError: User '%s' already has administrator privileges.%s\n", RED, target_username, RESET);
        free(accounts);
        return;
    }

    // --- Update role and rewrite file ---
    strcpy(accounts[target_index].role_str, "admin");

    file = fopen("accounts.txt", "w");
    if (file == NULL) {
        printf("%sError: Could not rewrite accounts file.%s\n", RED, RESET);
        free(accounts);
        return;
    }

    for (int i = 0; i < count; i++) {
        fprintf(file, "%s,%s,%s%s", accounts[i].user, accounts[i].pass, accounts[i].role_str, (i == count - 1) ? "" : "\n");
    }
    fclose(file);
    free(accounts);

    printf("%sUser '%s' now has administrator privileges.%s\n", GREEN, target_username, RESET);
}

/**
 * @brief The 'removeadmin' command removes administrator privileges from a user.
 * @details Only the root administrator can remove admin privileges. The target user must be an admin.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void handle_remove_admin(const int argc, char *argv[]) {
    (void)argc; // unused parameter
    const User *current_user = get_current_user();
    char *target_username = argv[1];

    // 1. Permission check: Only root can remove admin privileges.
    if (current_user->role != ROLE_ROOT) {
        printf("%sError: Permission denied. Only root can remove administrator privileges.%s\n", RED, RESET);
        return;
    }

    // --- Read all accounts into memory ---
    FILE *file = fopen("accounts.txt", "r");
    if (file == NULL) {
        printf("%sError: Could not open accounts file.%s\n", RED, RESET);
        return;
    }

    typedef struct { char user[MAX_USERNAME_LEN]; char pass[MAX_PASSWORD_LEN]; char role_str[20]; UserRole role; } Account;
    Account *accounts = NULL;
    int count = 0;
    char buffer[512];
    int target_index = -1;

    while (fgets(buffer, sizeof(buffer), file)) {
        buffer[strcspn(buffer, "\r\n")] = 0;
        if(strlen(buffer) == 0) continue;

        accounts = realloc(accounts, (count + 1) * sizeof(Account));
        sscanf(buffer, "%[^,],%[^,],%s", accounts[count].user, accounts[count].pass, accounts[count].role_str);

        if (strcmp(accounts[count].role_str, "root") == 0) accounts[count].role = ROLE_ROOT;
        else if (strcmp(accounts[count].role_str, "admin") == 0) accounts[count].role = ROLE_ADMIN;
        else accounts[count].role = ROLE_BASIC;

        if (strcmp(accounts[count].user, target_username) == 0) {
            target_index = count;
        }
        count++;
    }
    fclose(file);

    // 2. Check if user exists
    if (target_index == -1) {
        printf("%sError: User '%s' not found.%s\n", RED, target_username, RESET);
        free(accounts);
        return;
    }

    // 3. Prevent root from being demoted
    if (accounts[target_index].role == ROLE_ROOT) {
        printf("%sError: The root administrator's privileges cannot be removed.%s\n", RED, RESET);
        free(accounts);
        return;
    }

    // 4. Check if user is actually an admin
    if (accounts[target_index].role != ROLE_ADMIN) {
        printf("%sError: User '%s' does not have administrator privileges to remove.%s\n", YELLOW, target_username, RESET);
        free(accounts);
        return;
    }

    // --- Update role and rewrite file ---
    strcpy(accounts[target_index].role_str, "basic");

    file = fopen("accounts.txt", "w");
    if (file == NULL) {
        printf("%sError: Could not rewrite accounts file.%s\n", RED, RESET);
        free(accounts);
        return;
    }

    for (int i = 0; i < count; i++) {
        fprintf(file, "%s,%s,%s%s", accounts[i].user, accounts[i].pass, accounts[i].role_str, (i == count - 1) ? "" : "\n");
    }
    fclose(file);
    free(accounts);

    printf("%sUser '%s' no longer has administrator privileges.%s\n", GREEN, target_username, RESET);
}

#include "comhan.h"
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "commands.h"
#include "stdio.h"
#include "color_library.h"

/* global variables */
int g_comhan_running = 1;

/* command lookup table */
static const Command command_table[] = {
    {"help",      handle_help, 0,1, "help <command>"},
    {"version", handle_version, 0, 0, "version"},
    {"showdate", handle_show_date, 0, 0, "showdate"},
    {"setdate", handle_set_date, 1, 1, "setdate <MM-DD-YYYY>"},
    {"showtime", handle_show_time, 0, 0, "showtime"},
    {"exit", handle_terminate, 0, 0, "exit"},
    {"quit", handle_terminate, 0, 0, "quit"},
    {"createpcb", handle_create_pcb, 3, 3, "createpcb <name> <class> <priority>"},
    {"showallpcbs", handle_show_all_pcbs, 0, 0, "showallpcbs"},
    {"deletepcb", handle_delete_pcb, 1, 1, "deletepcb <name>"},
    {"blockpcb", handle_block_pcb, 1, 1, "blockpcb <name>"},
    {"unblockpcb", handle_unblock_pcb, 1, 1, "unblockpcb <name>"},
    {"suspendpcb", handle_suspend_pcb, 1, 1, "suspendpcb <name>"},
    {"resumepcb", handle_resume_pcb, 1, 1, "resumepcb <name>"},
    {"setpcbpriority", handle_set_pcb_priority, 2, 2, "setpcbpriority <name> <priority>"},
    {"showpcb", handle_show_pcb, 1, 1, "showpcb <name>"},
    {"showreadypcbs", handle_show_ready_pcbs, 0, 2, "showreadypcbs"},
    {"showblockedpcbs", handle_show_blocked_pcbs, 0, 0, "showblockedpcbs"},
    {"loadpcb", handle_load_pcbs, 3, 3, "loadpcb <name> <priority> <file_path>"},
    {"dispatchpcbs", handle_dispatch_pcbs, 0, 0, "dispatchpcbs"},
    {"clear", handle_clear, 0, 0, "clear"},
    {"ls", handle_view_directory, 0, 2, "ls <-l> <path>"},
    {"cd", handle_change_directory, 0, 1, "cd <path>"},
    {"mkdir", handle_create_directory, 1, 1, "mkdir <folder_name>"},
    {"rmdir", handle_remove_directory, 1, 1, "rmdir <folder_name>"},
    {"touch", handle_create_file, 1, 1, "touch <file_name>"},
    {"rm", handle_remove_file, 1, 1, "rm <file_name>"},
    {"adduser", handle_add_user, 3, 3, "adduser <username> <password> <role>"},
    {"removeuser", handle_remove_user, 1, 1, "removeuser <username>"},
    {"changepassword", handle_change_password, 1, 1, "changepassword <username>"},
    {"addadmin", handle_add_admin, 1, 1, "addadmin <username>"},
    {"removeadmin", handle_remove_admin, 1, 1, "removeadmin <username>"},
    {NULL,        NULL, 0, 0, NULL} // Sentinel to mark end of command table
};

/**
 * @brief function to dispatch the command to the appropriate handler.
 * @param argc number of arguments
 * @param argv array of argument strings
 */
static void dispatch_command(const int argc, char *argv[]) {
    if (argc == 0) {
        return;
    }
    char *command = argv[0];
    for (int i = 0; command_table[i].name != NULL; i++) {
        if (strcmp(command, command_table[i].name) == 0) {
            const int args_count = argc - 1;
            if (args_count >= command_table[i].min_args && args_count <= command_table[i].max_args) {
                command_table[i].handler(argc, argv);
            } else {
                printf("%sError: Invalid arguments for '%s'.%s\n", RED, command, RESET);
                printf("%sUsage: %s%s\n", MAGENTA, command_table[i].syntax, RESET);
            }
            return;
        }
    }
    printf("%sError: Unknown command '%s'. Type 'help' for available commands.%s\n", RED, command, RESET);
}

/**
 * @brief function to parse user input into command and arguments.
 * @param input user input string
 * @param argc pointer to argument count
 * @param argv array of argument strings
 */
static void parse_input(char *input, int *argc, char *argv[]) {
    *argc = 0;
    char *current_pos = input;
    bool in_quotes = false;
    char *token_start = NULL;

    while (*current_pos != '\0' && *argc < MAX_ARGS_COUNT) {
        if (isspace(*current_pos)) {
            if (!in_quotes) {
                if (token_start) {
                    *current_pos = '\0';
                    token_start = NULL;
                }
            }
        } else if (*current_pos == '"') {
            in_quotes = !in_quotes;
            if (token_start) {
                *current_pos = '\0'; // End current token
                token_start = NULL;
            }
        } else {
            if (!token_start) {
                token_start = current_pos;
                argv[(*argc)++] = token_start;
            }
        }
        current_pos++;
    }
}

/**
 * @brief Trims leading and trailing whitespace from a string in place.
 * @param str The string to trim.
*/
static void trim_whitespace(char *str) {
    char const *start = str;
    while (isspace(*start)) start++;
    if (start != str) memmove(str, start, strlen(start) + 1);
    char *end = str + strlen(str) - 1;
    while (end >= str && isspace(*end)) *end-- = '\0';
}

/**
 * @brief function to get user input and validate it.
 * @param buffer user input buffer
 * @return 0 if input is invalid, 1 if valid
 */
static int get_user_input(char *buffer) {
    if (!fgets(buffer, MAX_INPUT_LEN, stdin)) {
        perror("Error reading input.\n");
        return 0;
    }

    /* check for input length */
    if (buffer[strlen(buffer) - 1] != '\n') {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
        printf("%sError: Input too long. Please enter a command within %d characters.%s\n", RED, MAX_INPUT_LEN - 1, RESET);
        return 0;
    }

    buffer[strlen(buffer)-1] = '\0'; // strip newline
    trim_whitespace(buffer);
    return 1;
}

/**
 * @brief Runs the main command handler loop.
 * Displays prompt, gets input, parses, and dispatches commands.
 */
void comhan(void) {
    while (g_comhan_running) {
        char user_input[MAX_INPUT_LEN];
        char *argv[MAX_ARGS_COUNT + 1];
        int argc;
        printf("%s%s%s", BLUE, PROMPT, RESET);
        fflush(stdout); // ensure prompt is displayed before input

        /* read and validate user input */
        if (!get_user_input(user_input)) {
            /* if user input is invalid re-prompt the user to enter input again */
            continue;
        }

        /* If the user entered a blank line, just re-prompt without error */
        if (user_input[0] == '\0') {
            continue;
        }

        parse_input(user_input, &argc, argv);
        dispatch_command(argc, argv);
    }
}

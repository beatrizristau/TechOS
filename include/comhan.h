#ifndef COMHAN_H
#define COMHAN_H

/* Symbolic constant for maximum command input length */
#define MAX_INPUT_LEN 100
#define MAX_ARGS_COUNT 5  // Maximum number of arguments
#define MAX_ARGS_LEN MAX_INPUT_LEN  // Maximum length for the combined args string

/* Symbolic constant for the prompt */
#define PROMPT "TechOS> "

/*
 * argc: number of arguments
 * argv: array of arguments
 */
typedef void (*CommandHandler)(int argc, char *argv[]);

typedef struct {
    const char *name;
    CommandHandler handler;
    int min_args;
    int max_args;
    const char *syntax;
} Command;

extern int g_comhan_running;

void comhan(void);

#endif

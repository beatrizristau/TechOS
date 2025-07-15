#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/termios.h>
#include "color_library.h"

/**
 * @brief Global variable to store the TechOS date.
 * Initialized by init_techos_date().
*/
static techos_date current_techos_date;

/**
 * @brief Initializes the TechOS date to the current system date.
 */
void init_techos_date() {
    time_t current_datetime;
    struct tm local_date; // structure to hold the local time broken into components

    /* retrieve the current system datetime and store it in current_datetime */
    time(&current_datetime);

    /* Convert the system datetime to local time structure and store it in local_date */
    localtime_r(&current_datetime, &local_date);

    /* set the global TechOs date with the values from the local time structure */
    current_techos_date.day = local_date.tm_mday;
    current_techos_date.month = local_date.tm_mon + 1;
    current_techos_date.year = local_date.tm_year + 1900;
}

/**
 * @brief Gets the current TechOS date as a string.
 * @param buffer The buffer to store the date string (e.g., "MM-DD-YYYY").
 * @param size The size of the buffer.
 */
void get_techos_date_str(char *buffer, size_t size) {
    snprintf(buffer, size, "%02d-%02d-%d", current_techos_date.month, current_techos_date.day, current_techos_date.year);
}

/**
 * @brief Gets the current system time as a string.
 * @param buffer The buffer to store the time string (e.g., "HH:MM:SS").
 * @param size The size of the buffer.
*/
void get_current_time_str(char *buffer, size_t size) {
    time_t current_time;
    struct tm local_time;
    time(&current_time);
    struct tm const *prog_time = localtime_r(&current_time, &local_time);
    strftime(buffer, size, "%H:%M:%S", prog_time);
}

/**
 * @brief Checks if a year is a leap year.
 * @param year The year to check.
 * @return 1 if leap year, 0 otherwise.
*/
static int is_leap_year(const int year) {
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

/**
 * @brief Validates the logical correctness of date components.
 * @param month The month (1-12).
 * @param day The day.
 * @param year The year.
 * @return A DateValidationResult enum indicating the outcome.
 */
DateValidationResult validate_date_components(const int month, const int day, const int year) {
    /* validate month */
    if (month < 1 || month > 12) {
        return DATE_ERROR_INVALID_MONTH;
    }

    /* validate year */
    if (year < 1900) {
        return DATE_ERROR_INVALID_YEAR;
    }

    /* validate day based on month and leap year */
    int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (is_leap_year(year)) {
        days_in_month[2] = 29;
    }

    /* check if the day is valid for the given month */
    if (day < 1 || day > days_in_month[month]) {
        return DATE_ERROR_INVALID_DAY;
    }

    return DATE_VALID;
}

/**
 * @brief Sets the TechOS internal date after validating logical correctness.
 * @param month The month (1-12).
 * @param day The day.
 * @param year The year.
 * @return 1 if the date is logically valid (e.g., days in month), 0 otherwise.
*/
void set_techos_date(const int month, const int day, const int year) {
    /* update the global TechOS date */
    current_techos_date.day = day;
    current_techos_date.month = month;
    current_techos_date.year = year;
}

/**
 * @brief Trims leading and trailing whitespace from a string in place.
 * @param str The string to trim.
*/
void trim_whitespace(char *str) {
    char const *start = str;
    while (isspace(*start)) start++;
    if (start != str) memmove(str, start, strlen(start)+1);
    char *end = str + strlen(str) - 1;
    while (end >= str && isspace(*end)) *end-- = '\0';
}

/**
 * @brief function to print the contents of a file to stdout.
 * @param file_name name of the file to print
 */
void print_file(const char *file_name) {
    FILE *file = fopen(file_name, "r");
    if (!file) {
        printf("%sNo help available for command: %s --> %s%s", RED, file_name, strerror(errno), RESET);
        return;
    }
    char buf[512];
    while (fgets(buf, sizeof(buf), file)) {
        fputs(buf, stdout);
    }
    fclose(file);
}

/**
 * @brief function to validate p_name.
 * This function checks if the name is not NULL, has a length between 1 and 8,
 * and contains only alphanumeric characters. First character must be a letter.
 * @param name the name to be validated
 * @return returns 1 if valid, 0 otherwise
 */
int validate_name(const char *name) {
    if (!name || strlen(name) == 0 || strlen(name) > 8) {
        return 0;
    }

    /* First character must be a letter */
    if (!isalpha(name[0])) {
        return 0;
    }
    return 1;
}

/**
 * @brief function to validate priority.
 * This function checks if the string is a valid integer between 0 and 9.
 * @param str the string to be validated
 * @param priority pointer to store the parsed priority value
 * @return returns 1 if valid, 0 otherwise
*/
int validate_priority(const char *str, int *priority) {
    char *endptr;
    const long val = strtol(str, &endptr, 10);
    if (*endptr != '\0' || val < 0 || val > 9) {
        return 0;
    }
    *priority = (int)val;
    return 1;
}

/**
 * @brief function to validate p_class.
 * This function checks if the string is a valid integer 0 (system) or 1 (application).
 * @param str the string to be validated
 * @param p_class pointer to store the parsed class value
 * @return returns 1 if valid, 0 otherwise
*/
int validate_class(const char *str, int *p_class) {
    char *endptr;
    const long val = strtol(str, &endptr, 10);
    if (*endptr != '\0' || val < 0 || val > 1) {
        return 0;
    }
    *p_class = (int)val;
    return 1;
}

/**
 * @brief function to validate a file path.
 * This function checks if the path is not NULL, has a length between 1 and 8,
 * and contains only alphanumeric characters.
 * @param file_path the file path to be validated
 * @return returns 1 if valid, 0 otherwise
 */
int validate_file_path(const char *file_path) {
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "%s", file_path);
    if (access(full_path, R_OK) != 0) {
        printf("%sError: No file present at %s --> %s%s\n", RED, full_path, strerror(errno), RESET);
        return 0;
    }
    return 1;
}

/**
 * @brief Helper function to check if a directory is empty.
 * @param path The path to the directory.
 * @return True if the directory is empty (contains only '.' and '..'), false otherwise.
 */
int is_directory_empty(const char *path) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        return 1; // If we can't open it, we can't check it. Error handled by caller.
    }

    const struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            closedir(dir);
            return 0; // Found an entry other than '.' or '..'
        }
    }

    closedir(dir);
    return 0; // Directory is empty
}

/**
 * @brief Reads a line from stdin without echoing characters. Used for password input.
 * @param buffer The buffer to store the password in.
 * @param size The size of the buffer.
 */
void get_secure_password(char *buffer, size_t size) {
    struct termios old_term, new_term;
    tcgetattr(STDIN_FILENO, &old_term);
    new_term = old_term;
    new_term.c_lflag &= ~(ECHO); // Disable character echoing.
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

    fgets(buffer, size, stdin);
    buffer[strcspn(buffer, "\n")] = 0; // Remove trailing newline.

    tcsetattr(STDIN_FILENO, TCSANOW, &old_term); // Restore terminal settings.
}
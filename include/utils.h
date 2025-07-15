#ifndef UTILS_H
#define UTILS_H

#include <time.h>

/**
 * @brief Enum to represent the result of a date validation check.
*/
typedef enum {
    DATE_VALID,
    DATE_ERROR_INVALID_MONTH,
    DATE_ERROR_INVALID_YEAR,
    DATE_ERROR_INVALID_DAY
} DateValidationResult;

/* Structure to hold the current system/program date */
typedef struct {
    int day;
    int month; // 1-12
    int year;
} techos_date;

void init_techos_date(void);
void get_techos_date_str(char *buffer, size_t size);
void get_current_time_str(char *buffer, size_t size);
DateValidationResult validate_date_components(int month, int day, int year);
void set_techos_date(int month, int day, int year);
void trim_whitespace(char *str);
void print_file(const char *file_name);
int validate_name(const char *name);
int validate_priority(const char *str, int *priority);
int validate_class(const char *str, int *p_class);
int validate_file_path(const char *file_name);
int is_directory_empty(const char *path);
void get_secure_password(char *buffer, size_t size);

#endif

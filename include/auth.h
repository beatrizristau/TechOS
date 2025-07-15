#ifndef AUTH_H
#define AUTH_H

#include <stdbool.h>

#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define ACCOUNTS_FILE "accounts.txt"

// Defines the different roles a user can have.
typedef enum {
    ROLE_BASIC,
    ROLE_ADMIN,
    ROLE_ROOT,
    ROLE_UNKNOWN // For error handling
} UserRole;

// Represents the currently logged-in user.
typedef struct {
    char username[MAX_USERNAME_LEN];
    UserRole role;
} User;

bool handle_login(void);
const User* get_current_user(void);
UserRole get_current_user_role(void);
const char* get_current_user_name(void);

#endif //AUTH_H

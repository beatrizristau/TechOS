#include "auth.h"
#include "color_library.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "utils.h"

// Stores the currently logged-in user's data.
static User g_current_user = { "guest", ROLE_UNKNOWN };

/**
 * @brief Converts a role string to its corresponding UserRole enum.
 * @param role_str The string to convert ("basic", "admin", "root").
 * @return The matching UserRole enum, or ROLE_UNKNOWN if not found.
 */
static UserRole string_to_role(const char *role_str) {
    if (strcmp(role_str, "root") == 0) return ROLE_ROOT;
    if (strcmp(role_str, "admin") == 0) return ROLE_ADMIN;
    if (strcmp(role_str, "basic") == 0) return ROLE_BASIC;
    return ROLE_UNKNOWN;
}

/**
 * @brief Validates user credentials against the accounts file.
 * @param username The username to validate.
 * @param password The password to validate.
 * @return true if credentials are valid, false otherwise.
 */
static bool validate_credentials(const char *username, const char *password) {
    FILE *file = fopen(ACCOUNTS_FILE, "r");
    if (file == NULL) {
        printf("%sError: Could not open accounts file '%s'.%s\n", RED, ACCOUNTS_FILE, RESET);
        printf("%sPlease ensure the file exists and contains valid credentials.%s\n", YELLOW, RESET);
        return false;
    }

    char line[MAX_USERNAME_LEN + MAX_PASSWORD_LEN + 2];
    bool success = false;
    while (fgets(line, sizeof(line), file)) {
        // Handle both LF (\n) and CRLF (\r\n) line endings.
        line[strcspn(line, "\r\n")] = 0;

        // Manually split the string at the comma to be more robust.
        char *user_part = strtok(line, ",");
        char *pass_part = strtok(NULL, ",");
        char *role_part = strtok(NULL, ",");

        if (user_part == NULL || pass_part == NULL || role_part == NULL) {
            continue; // Skip any malformed lines.
        }

        // Trim whitespace to handle potential formatting errors.
        trim_whitespace(user_part);
        trim_whitespace(pass_part);
        trim_whitespace(role_part);

        // Now, compare the clean, validated credentials.
        if (strcmp(username, user_part) == 0 && strcmp(password, pass_part) == 0) {
            // Credentials are correct, store user info.
            strncpy(g_current_user.username, user_part, MAX_USERNAME_LEN - 1);
            g_current_user.username[MAX_USERNAME_LEN - 1] = '\0';
            g_current_user.role = string_to_role(role_part);
            success = true;
            break;
        }
    }

    fclose(file);
    return success;
}

/**
 * @brief Handles the user login process.
 * @details Prompts for username and password, validates them against the accounts file,
 * and allows retries on failure.
 * @return true if login is successful, false if the user chooses to exit.
 */
bool handle_login(void) {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    int attempts = 0;
    const int max_attempts = 3;

    while (attempts < max_attempts) {
        printf("----------------------------- TechOS Login --------------------------------\n");
        printf("%sUsername: %s", BLUE, RESET);
        fflush(stdout);

        if (!fgets(username, sizeof(username), stdin)) {
            return false; // Handle EOF or input error.
        }
        username[strcspn(username, "\n")] = 0;

        printf("%sPassword: %s", BLUE, RESET);
        fflush(stdout);
        get_secure_password(password, sizeof(password));
        printf("---------------------------------------------------------------------------\n");

        if (validate_credentials(username, password)) {
            printf("%sLogin successful. Welcome, %s!%s\n", GREEN, g_current_user.username, RESET);
            return true;
        } else {
            attempts++;
            printf("%sError: Invalid username or password.%s\n", RED, RESET);
            if (attempts < max_attempts) {
                printf("%sYou have %d attempt(s) remaining.%s\n", YELLOW, max_attempts - attempts, RESET);
            }
        }
    }

    // If the loop completes, the user has failed all attempts.
    printf("%sMaximum login attempts reached. Exiting TechOS.%s\n", RED, RESET);
    return false;
}

/**
 * @brief Gets the currently logged-in user's data.
 * @return A constant pointer to the current user's struct.
 */
const User* get_current_user(void) {
    return &g_current_user;
}

/**
 * @brief Helper function to get the current user's role.
 * @return The UserRole enum for the current user.
 */
UserRole get_current_user_role(void) {
    return g_current_user.role;
}

/**
 * @brief Helper function to get the current user's name.
 * @return A constant string of the current user's name.
 */
const char* get_current_user_name(void) {
    return g_current_user.username;
}
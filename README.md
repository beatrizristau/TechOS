# TechOS

# Projection Information
- **Project Name:** TechOS
- **Course:** CS 450 - Operating System Structures
- **Institution:** West Virginia University Institute of Technology
- **Authors:** Beatriz Ristau & Tommi Kenneda
- **Completion Date:** May 6, 2021

# Project Overview
TechOS is a lightweight operating system implementation developed as part of CS 450 (Operating
System Structures) at West Virginia University Institute of Technology. The project showcases
core operating system concepts such as process management, memory management, file systems, and
user interfaces. The project is divided into several modules, each focusing on a specific
aspect of operating system design.

# Module R1 - Command Line Interface Foundation
This module establishes the fundamental user interface framework for TechOS. This module 
implements the Command Handler (COMHAN) that serves as the primary interaction point 
between users and the operating system. The Command Handler processes user input, executes commands, 
and manages the command history. It is designed to be extensible, allowing for the addition of new commands and features in future
modules.

## Input Validation and Error Handling
In this module, a input validation and error handling mechanism is implemented to ensure that user inputs are correctly formatted and valid.
This includes checking for valid command syntax, ensuring that required parameters are provided, and handling errors gracefully
without crashing the system. The Command Handler provides feedback to users when invalid inputs are detected, guiding
them to correct usage.

### Command Recognition and Case Sensitivity
The system implements **case-sensitive** command recognition, which means that users must type commands exactly as specified.
When a user types a command, the Command Handler checks the input against a predefined list of valid commands.
If the command is recognized, it is executed; otherwise, an error message is displayed. This
ensures that users are aware of the correct command syntax and helps prevent accidental command execution.

### Whitespace Handling and Input Normalization
The Command Handler normalizes user input by removing leading and trailing whitespace. This allows users to enter
commands without worrying about extra spaces at the beginning or end of their input. The system also supports
commands with multiple spaces between words, treating them as a single space. This flexibility enhances user experience
by allowing more natural input without strict formatting requirements.

### Invalid Command Detection and Error Messages
When a user enters an invalid command, the Command Handler detects this and provides a clear error message.
The error message includes the command that was attempted and a suggestion to use the `help` command for assistance.
This helps users understand what went wrong and how to correct their input. 

### Argument Validation and Parameter Checking
The Command Handler checks for the presence of required parameters for commands that require them.
If the parameter is incorrect, the system provides an error message indicating the expected format.
This validation ensures that commands are executed with the correct parameters, preventing errors during execution.

### Missing Argument Detection
For commands that require arguments, the Command Handler checks if the user has provided the necessary parameters.
If a required argument is missing, the system prompts the user with an error message indicating the expected
syntax. This helps users understand how to properly use the command and prevents confusion.

### Input Length and Buffer Management
The Command Handler limits the length of user input to prevent buffer overflow and ensure system stability. 
Max user input length is set to 100 characters. If a user attempts to enter a command that 
exceeds the maximum allowed length, the system provides an error message indicating that the input is too long. 
This prevents potential security vulnerabilities and ensures that the system remains responsive to user commands.

## Available Commands

### help
- **Purpose:** Provides documentation for all system commands, supporting both general 
overview and detailed command-specific information.
- **Syntax:** 
    `help`
    `help [command]`
- **Implementation Details:** 
    - Displays a list of all available commands when called without arguments.
    - When called with a specific command, it provides detailed information about that command.
    - Uses a simple text-based interface to present help information.
- **Usages Example:**
```
TechOS> help
TechOS Help Summary
------------------------------------
    help [command]        - Display a summary of all commands or detailed help for a specific command.
    version               - Show current version, authors, and completion date.
    showdate              - Display the current system date.
    setdate MM-DD-YYYY    - Change the system date to the specified value.
    showtime              - Display the current system time.
    exit, quit            - Terminate TechOS (with confirmation).

TechOS> help setdate
Command: setdate
Purpose: Change the system date with validation
Syntax: setdate [MM-DD-YYYY] or setdate (for prompted input)
Example: setdate 12-25-2021
Validation: Ensures proper days per month and valid date ranges
```

### version
- **Purpose:** Displays the current version of TechOS, along with author information and the date of completion.
- **Syntax:**
    `version`
- **Implementation Details:**
    - Outputs the version number, author names, and completion date in a formatted manner.
- **Usages Example:**
```
TechOS> version
TechOS Version: 1.0
Authors: Beatriz Ristau & Tommi Kenneda
Completion Date: May  6 2021
```

### showdate
- **Purpose:** Displays the current system date using system time functions and formatted output.
- **Syntax:**
    `showdate`
- **Implementation Details:**
    - Retrieves and displays the current date in a user-friendly format.
- **Usages Example:**
```
TechOS> showdate
Current TechOS Date: 07-06-2021
```

### setdate
- **Purpose:** Allows users to change the system date with validation to ensure correctness.
- **Syntax:**
    `setdate MM-DD-YYYY`
- **Implementation Details:**
    - Accepts a date in the format MM-DD-YYYY.
    - Validates the date to ensure it is within acceptable ranges (e.g., no February 30th).
    - If no date is provided, error message is displayed with the correct syntax.
    - If the date is valid, it updates the system date and confirms the change.
- **Usages Example:**
```
TechOS> setdate
Error: Missing date argument. Syntax: setdate MM-DD-YYYY

TechOS> setdate 02-30-2021
Error: Invalid date value (e.g., day 30 is invalid for month 2 in year 2021).

TechOS> setdate july 8 2021
Error: Invalid date format. Please use MM-DD-YYYY.

TechOS> setdate 7-21-2020
TechOS date successfully changed to: 07-21-2020
```

### showtime
- **Purpose:** Displays the current system time using system time functions and formatted output.
- **Syntax:**
    `showtime`
- **Implementation Details:**
    - Retrieves and displays the current time in a user-friendly format.
    - Displays in 24-hour format (HH:MM:SS).
- **Usages Example:**
```
TechOS> showtime
Current System Time: 12:42:03
```

### exit, quit
- **Purpose:** Terminates the TechOS session with a confirmation prompt.
- **Syntax:**
    `exit`
    `quit`
- **Implementation Details:**
    - Prompts the user for confirmation before exiting.
    - If confirmed, the session is terminated gracefully.
    - If the user chooses not to exit, the system returns to the command prompt.
- **Usages Example:**
```
TechOS> quit
Are you sure you want to exit TechOS? (yes/no): yes
Performing TechOS cleanup...
Shutting down TechOS. Goodbye!

TechOS> exit
Are you sure you want to exit TechOS? (yes/no): no
Termination cancelled.
TechOS> 
```

# Module R2 - Process Management

## Module Overview
This module implements the core process management functionalities of TechOS, 
including process creation, scheduling, and termination. It introduces a simple process 
control block (PCB) structure to manage process states and attributes. 
The module also implements a round-robin scheduling algorithm to ensure fair CPU time 
allocation among processes.





gcc src/execute.c -o execute

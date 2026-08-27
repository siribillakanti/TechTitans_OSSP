#include <stdio.h>
#include <string.h>

int main() {
    char command[100];

    while (1) {
        // Display prompt
        printf("myshell> ");
        fflush(stdout);

        // Read user input
        if (fgets(command, sizeof(command), stdin) == NULL) {
            printf("\nExiting shell...\n");
            break;
        }

        // Remove newline character
        command[strcspn(command, "\n")] = '\0';

        // Check exit condition
        if (strcmp(command, "exit") == 0) {
            printf("Exiting shell...\n");
            break;
        }

        // Process command
        printf("You entered: %s\n", command);
    }

    return 0;
}


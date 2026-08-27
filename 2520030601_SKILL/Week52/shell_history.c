#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#define INITIAL_CAPACITY 16

/* -------------------- Dynamic Input Buffer -------------------- */

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
    size_t cursor;
} InputBuffer;

void initBuffer(InputBuffer *buffer) {
    buffer->capacity = INITIAL_CAPACITY;
    buffer->data = malloc(buffer->capacity);
    buffer->length = 0;
    buffer->cursor = 0;

    if (buffer->data == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    buffer->data[0] = '\0';
}

void resizeBuffer(InputBuffer *buffer) {
    buffer->capacity *= 2;

    char *temp = realloc(buffer->data, buffer->capacity);

    if (temp == NULL) {
        free(buffer->data);
        perror("realloc");
        exit(EXIT_FAILURE);
    }

    buffer->data = temp;
}

void insertChar(InputBuffer *buffer, char c) {
    if (buffer->length + 1 >= buffer->capacity) {
        resizeBuffer(buffer);
    }

    memmove(
        &buffer->data[buffer->cursor + 1],
        &buffer->data[buffer->cursor],
        buffer->length - buffer->cursor + 1
    );

    buffer->data[buffer->cursor] = c;
    buffer->length++;
    buffer->cursor++;
}

void deleteChar(InputBuffer *buffer) {
    if (buffer->cursor == 0) {
        return;
    }

    memmove(
        &buffer->data[buffer->cursor - 1],
        &buffer->data[buffer->cursor],
        buffer->length - buffer->cursor + 1
    );

    buffer->length--;
    buffer->cursor--;
}

void clearBuffer(InputBuffer *buffer) {
    buffer->length = 0;
    buffer->cursor = 0;
    buffer->data[0] = '\0';
}

void setBuffer(InputBuffer *buffer, const char *text) {
    size_t length = strlen(text);

    while (length + 1 >= buffer->capacity) {
        resizeBuffer(buffer);
    }

    strcpy(buffer->data, text);

    buffer->length = length;
    buffer->cursor = length;
}

void freeBuffer(InputBuffer *buffer) {
    free(buffer->data);
    buffer->data = NULL;
}

/* -------------------- Command History -------------------- */

typedef struct HistoryNode {
    char *command;
    struct HistoryNode *prev;
    struct HistoryNode *next;
} HistoryNode;

typedef struct {
    HistoryNode *head;
    HistoryNode *tail;
    HistoryNode *current;
} History;

void initHistory(History *history) {
    history->head = NULL;
    history->tail = NULL;
    history->current = NULL;
}

void addHistory(History *history, const char *command) {
    if (strlen(command) == 0) {
        return;
    }

    HistoryNode *node = malloc(sizeof(HistoryNode));

    if (node == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    node->command = malloc(strlen(command) + 1);

    if (node->command == NULL) {
        free(node);
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strcpy(node->command, command);

    node->prev = history->tail;
    node->next = NULL;

    if (history->tail != NULL) {
        history->tail->next = node;
    } else {
        history->head = node;
    }

    history->tail = node;
    history->current = NULL;
}

void freeHistory(History *history) {
    HistoryNode *current = history->head;

    while (current != NULL) {
        HistoryNode *next = current->next;

        free(current->command);
        free(current);

        current = next;
    }

    history->head = NULL;
    history->tail = NULL;
    history->current = NULL;
}

/* -------------------- Terminal Raw Mode -------------------- */

struct termios originalTermios;

void disableRawMode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermios);
}

void enableRawMode(void) {
    if (tcgetattr(STDIN_FILENO, &originalTermios) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    atexit(disableRawMode);

    struct termios raw = originalTermios;

    raw.c_lflag &= ~(ECHO | ICANON);

    raw.c_iflag &= ~(IXON);

    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

/* -------------------- Screen Handling -------------------- */

void refreshLine(InputBuffer *buffer) {
    printf("\r\033[2K");
    printf("myshell> %s", buffer->data);

    /* Move cursor back to the correct position */
    size_t charactersAfterCursor =
        buffer->length - buffer->cursor;

    if (charactersAfterCursor > 0) {
        printf("\033[%zuD", charactersAfterCursor);
    }

    fflush(stdout);
}

/* -------------------- Command Processing -------------------- */

int processCommand(const char *command) {
    if (strcmp(command, "hello") == 0) {
        printf("\nHello! Welcome to MyShell.\n");
    }
    else if (strcmp(command, "help") == 0) {
        printf("\nAvailable commands:\n");
        printf("  hello - Display greeting\n");
        printf("  help  - Display available commands\n");
        printf("  exit  - Exit the shell\n");
    }
    else if (strlen(command) == 0) {
        return 1;
    }
    else {
        printf("\nUnknown command: %s\n", command);
    }

    return 1;
}

/* -------------------- Main -------------------- */

int main(void) {
    InputBuffer buffer;
    History history;

    initBuffer(&buffer);
    initHistory(&history);

    char *savedInput = NULL;

    enableRawMode();

    printf("MyShell - Skill 2\n");
    printf("Use Up/Down arrows for command history.\n\n");

    while (1) {
        printf("myshell> ");
        fflush(stdout);

        clearBuffer(&buffer);

        while (1) {
            char c;

            if (read(STDIN_FILENO, &c, 1) != 1) {
                break;
            }

            /* Enter key */
            if (c == '\n' || c == '\r') {
                printf("\n");

                if (strlen(buffer.data) > 0) {
                    addHistory(&history, buffer.data);

                    if (strcmp(buffer.data, "exit") == 0) {
                        printf("Exiting shell...\n");
                        goto cleanup;
                    }

                    processCommand(buffer.data);
                }

                break;
            }

            /* Ctrl+D */
            if (c == 4) {
                printf("\nExiting shell...\n");
                goto cleanup;
            }

            /* Backspace */
            if (c == 127 || c == 8) {
                if (buffer.cursor > 0) {
                    deleteChar(&buffer);
                    refreshLine(&buffer);
                }
                continue;
            }

            /* Escape sequence */
            if (c == '\033') {
                char seq[2];

                if (read(STDIN_FILENO, &seq[0], 1) != 1)
                    continue;

                if (seq[0] != '[')
                    continue;

                if (read(STDIN_FILENO, &seq[1], 1) != 1)
                    continue;

                /* Up arrow */
                if (seq[1] == 'A') {
                    if (history.current == NULL) {
                        if (history.tail != NULL) {
                            if (savedInput != NULL) {
                                free(savedInput);
                            }

                            savedInput = malloc(strlen(buffer.data) + 1);

                            if (savedInput != NULL) {
                                strcpy(savedInput, buffer.data);
                            }

                            history.current = history.tail;
                            setBuffer(&buffer,
                                      history.current->command);
                        }
                    }
                    else if (history.current->prev != NULL) {
                        history.current = history.current->prev;
                        setBuffer(&buffer,
                                  history.current->command);
                    }

                    refreshLine(&buffer);
                }

                /* Down arrow */
                else if (seq[1] == 'B') {
                    if (history.current != NULL) {
                        if (history.current->next != NULL) {
                            history.current = history.current->next;

                            setBuffer(&buffer,
                                      history.current->command);
                        }
                        else {
                            history.current = NULL;

                            if (savedInput != NULL) {
                                setBuffer(&buffer, savedInput);
                            }
                            else {
                                clearBuffer(&buffer);
                            }

                            free(savedInput);
                            savedInput = NULL;
                        }

                        refreshLine(&buffer);
                    }
                }

                /* Right arrow */
                else if (seq[1] == 'C') {
                    if (buffer.cursor < buffer.length) {
                        buffer.cursor++;
                        refreshLine(&buffer);
                    }
                }

                /* Left arrow */
                else if (seq[1] == 'D') {
                    if (buffer.cursor > 0) {
                        buffer.cursor--;
                        refreshLine(&buffer);
                    }
                }

                continue;
            }

            /* Normal printable character */
            if (c >= 32 && c <= 126) {
                insertChar(&buffer, c);
                refreshLine(&buffer);
            }
        }
    }

cleanup:
    disableRawMode();

    free(savedInput);
    freeBuffer(&buffer);
    freeHistory(&history);

    return 0;
}

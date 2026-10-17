
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(void)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        execlp("ls", "ls", "-la", (char *)NULL);
        perror("execlp");
        exit(1);
    }

    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status))
        printf("Utility finished with exit code %d.\n",
               WEXITSTATUS(status));
    else
        printf("Utility did not exit normally.\n");

    return 0;
}


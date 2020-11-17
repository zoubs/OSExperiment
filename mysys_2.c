#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
int mysys(const char *cmdstring)
{
    if (cmdstring == NULL)
    {
        return -1;
    }
    int status;
    pid_t pid;
    pid = fork();
    if (pid < 0)
    {
        status = -1;
    }
    else if (pid == 0)
    {
        execl("/bin/sh", "sh", "-c", cmdstring, NULL);
        exit(127);
    }
    else
    {
        while (waitpid(pid, &status, 0) < 0)
        {
            if (errno != EINTR)
            {
                status = -1;
                break;
            }
        }
    }
    return status;
}

void main(int argc, char **argv)
{
    printf("--------------------------------------------------\n");
    mysys("echo HELLO WORLD");
    printf("--------------------------------------------------\n");
    mysys("ls /");
    printf("--------------------------------------------------\n");
}

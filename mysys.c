#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define MAX_LEN 256
#define NUM 40

int mysys(const char *cmdstring)
{
    char str[MAX_LEN];
    strcpy(str, cmdstring);
    char *str_temp = str; //strsep参数是指针变量，不能是sz数组名(指针常量)
    char *tmp;
    char *argv[NUM];
    char s[NUM][NUM];
    tmp = strsep(&str_temp, " ");
    int i = 0;
    while (tmp != NULL)
    {
        if (*tmp)
        {
            strcpy(s[i], tmp);
            argv[i] = s[i];
        }
        //	printf("argv[%d] = %s\n", i, argv[i]);
        i++;
        tmp = strsep(&str_temp, " ");
    }
    argv[i] = NULL;
    pid_t pid;
    pid = fork();
    if (pid < 0)
    {
        printf("fork error!\n");
        return -1;
    }
    else if (pid == 0)
    {
        int error = execvp(argv[0], argv);
        if (error < 0)
        {
            perror("execvp");
            return 127;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        wait(NULL);
    }
    return 0;
}
int main(int argc, char *argv[])
{
    printf("--------------------------------------------------\n");
    mysys("echo HELLO WORLD");
    printf("--------------------------------------------------\n");
    mysys("ls /");
    printf("--------------------------------------------------\n");
    return 0;
}

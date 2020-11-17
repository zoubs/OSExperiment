#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define MAX_LEN 256
#define NUM 40

int mysys(const char *cmdstring)
{
    char str[MAX_LEN];
    strcpy(str, cmdstring);
    char *str_temp = str; //strsep参数是指针变量，不能是数组名(指针常量)
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
            i++;
        }
        tmp = strsep(&str_temp, " ");
    }
    argv[i] = NULL;
    if (!strcmp(argv[0], "exit"))
    {
        return 1;
    }
    else if (!strcmp(argv[0], "cd"))
    {
        int res = chdir(argv[1]);
        if (res == -1)
        {
            printf("cd: No such path %s\n", argv[1]);
        }
        return 2;
    }
    pid_t pid;
    int fd;
    pid = fork();
    if (pid < 0)
    {
        printf("fork error!\n");
        return -1;
    }
    else if (pid == 0)
    {
        --i;
        for (; i >= 0; i--)
        {
            if (!strcmp(argv[i], ">")) // > 左右都有空格
            {
                fd = open(argv[i + 1], O_CREAT | O_RDWR, 0666);
                if (fd < 0)
                {
                    printf("file error\n");
                    exit(100);
                }
                dup2(fd, 1);
                close(fd);
                argv[i] = NULL;
                break;
            }
            else if (argv[i][0] == '>') // > 左边有空格，右边无
            {
                char file_name[MAX_LEN];
                strcpy(file_name, argv[i] + 1);
                printf("%s\n", file_name);
                fd = open(file_name, O_CREAT | O_RDWR, 0666);
                if (fd < 0)
                {
                    printf("file error\n");
                    exit(100);
                }
                dup2(fd, 1);
                close(fd);
                argv[i] = NULL;
                break;
            }
            if (!strcmp(argv[i], "<")) // < 左右都有空格
            {
                fd = open(argv[i + 1], O_CREAT | O_RDWR, 0666);
                if (fd < 0)
                {
                    printf("file error\n");
                    exit(100);
                }
                dup2(fd, 0);
                close(fd);
                argv[i] = NULL;
                break;
            }
            else if (argv[i][0] == '<') // < 左边有空格，右边无
            {
                char file_name[MAX_LEN];
                strcpy(file_name, argv[i] + 1);
                printf("%s\n", file_name);
                fd = open(file_name, O_CREAT | O_RDWR, 0666);
                if (fd < 0)
                {
                    printf("file error\n");
                    exit(100);
                }
                dup2(fd, 0);
                close(fd);
                argv[i] = NULL;
                break;
            }
        }
        //for()
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

int main(int argc, char* argv[])
{
    char buf[MAX_LEN];
    char tmp[6];
    int len;
    printf("Please Input Command:\n> ");
    while(fgets(buf,MAX_LEN ,stdin))
    {
        len = strlen(buf);
        if(buf[len-1] == '\n')
        {
            buf[len-1] = '\0';
        }
        int type = mysys(buf);
        if(type == 1)//exit
        {
            exit(0);
        }
        else if(type == 2) //cd
        {
            //    
        }
            printf("Please Input Command:\n> ");
        }
        return 0;
}

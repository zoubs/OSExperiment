#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define MAX_LEN 256
#define NUM 40
#define PID_NUM 20
int count_line = 0;
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

int my_pipe(const char *cmdstring)
{
    char str[MAX_LEN];
    strcpy(str, cmdstring);
    char *str_temp = str; //strsep参数是指针变量，不能是数组名(指针常量)
    char *tmp;
    char *argv[NUM];
    char *cmd[NUM]; //指向多个命令字符串
    char cmd_str[NUM][NUM];
    char s[NUM][NUM];
    tmp = strsep(&str_temp, "|");
    int i = 0;
    while (tmp != NULL)
    {
        if (*tmp)
        {
            int len_tmp = strlen(tmp);
            int not_blank = 0;
            for (; not_blank < len_tmp; not_blank++)
            {
                if (tmp[not_blank] != ' ')
                {
                    break;
                }
            }
            strcpy(cmd_str[i], tmp + not_blank);
            cmd[i] = cmd_str[i];
            i++;
        }
        tmp = strsep(&str_temp, "|");
    }
    cmd[i] == NULL; //此时i==count_line
    //pid_t pid[PID_NUM];
    //for()
    //int i;
    pid_t pid;
    int fd[2];
    pipe(fd);
    pid = fork();
    if (pid == 0)
    {
        dup2(fd[1], 1);
        close(fd[1]);
        close(fd[0]);
        mysys(cmd[0]);
        exit(0);
    }
    else if (pid > 0)
    {
        waitpid(pid, NULL, 0);
        dup2(fd[0], 0);
        close(fd[1]);
        close(fd[0]);
        mysys(cmd[1]);
        exit(0);
    }
    else
    {
        printf("fork error\n");
    }
    return 0;
}

int main(int argc, char *argv[])
{
    char buf[MAX_LEN];
    char tmp[6];
    int len, i;
    printf("Please Input Command:\n");
    while (fgets(buf, MAX_LEN, stdin))
    {
        count_line = 0;
        len = strlen(buf);
        for (i = 0; i < len; i++)
        {
            if (buf[i] == '|')
            {
                count_line++;
            }
        }
        if (buf[len - 1] == '\n')
        {
            buf[len - 1] = '\0';
        }
        if (count_line == 0)
        {
            int type = mysys(buf);
            if (type == 1) //exit
            {
                exit(0);
            }
        }
        else if (count_line > 0)
        {
            pid_t pid = fork();
            if (pid == 0)
            {
                my_pipe(buf);
                exit(0);
            }
            else if (pid > 0)
            {
                wait(0);
            }
            else
            {
                printf("my_pipe fork error\n");
            }
        }
        printf("Please Input Command:\n");
    }
    return 0;
}

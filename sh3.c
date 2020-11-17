#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_LEN 256
#define MAX_NUM 100

char *home;
char *dir;
int RecoverPipeIn;
int RecoverPipeOut;
int count_line = 0;
int fdin, fdout;
int fd[2], fd_temp[2];
int flag = -1;
void split(char *src, int *argc, char **argv)
{
    char code[MAX_LEN];
    int count = 0; // N.O. of arguments
    char *next = NULL;
    char *rest = code;
    strcpy(code, src);
    argv[count++] = code;
    while (next = strchr(rest, ' '))
    {
        next[0] = '\0';
        rest = next + 1;
        if (rest[0] != '\0' && rest[0] != ' ')
            argv[count++] = rest;
        if (count + 2 > MAX_NUM)
            return;
    }
    argv[count++] = NULL;
    *argc = count - 1;
}
int mysys(const char *cmdstring)
{
    pid_t pid;
    int status = -1;
    if (cmdstring == NULL)
        return 1;
    if ((pid = fork()) < 0)
        status = -1;
    else if (pid == 0)
    {
        dup2(fdin, 0);
        dup2(fdout, 1);
        close(fdin);
        close(fdout);
        execl("/bin/sh", "sh", "-c", cmdstring, (char *)0);
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
int judge_command(char *buff)
{
    if (buff[0] == '\0')
        return 0;
    char code[MAX_LEN];
    strcpy(code, buff);
    char *next = strchr(code, ' ');
    if (next != NULL)
        next[0] = '\0';
    if (strcmp(code, "cd") == 0)
        return 1;
    else if (strcmp(code, "exit") == 0)
    {
        exit(-1);
    }
    else
        return 0;
}
int cd(char *buff)
{
    int argc = 0;
    char *argv[MAX_NUM]; // no more than 100 arguments
    int count = 0;       // N.O. of arguments
    split(buff, &argc, argv);
    count = argc;
    if (count == 1)
    {
        chdir(home);
        dir = getcwd(NULL, 0);
    }
    else
    {
        int res = chdir(argv[count - 1]);
        dir = getcwd(NULL, 0);
        if (res == -1)
        {
            printf("cd: No such path %s\n", argv[count - 1]);
            return -1;
        }
    }
    return 0;
}
int go(char *buff)
{
    int type = judge_command(buff);
    if (type == 0)
        mysys(buff);
    else if (type == 1)
        cd(buff);
    else if (type == -1)
        return -1;
    return 1;
}
void strip(char *s)
{
    size_t i;
    size_t len = strlen(s);
    size_t offset = 0;
    for (i = 0; i < len; ++i)
    {
        char c = s[i];
        if (c == 0x0d || c == 0x0a)
            ++offset;
        else
            s[i - offset] = c;
    }
    s[len - offset] = '\0';
}
void strip_char(char *s, char bad)
{
    int i;
    int len = strlen(s);
    int offset = 0;
    for (i = 0; i < len; ++i)
    {
        char c = s[i];
        if (c == bad)
            ++offset;
        else
            s[i - offset] = c;
    }
    s[len - offset] = '\0';
}
void strip_dup(char *s)
{
    size_t i;
    size_t len = strlen(s);
    for (i = 0; i < len; ++i)
    {
        char c = s[i];
        if (c == '<' || c == '>')
            s[i] = '\0';
    }
}
void strip_pipe(char *s)
{
    size_t i;
    size_t len = strlen(s);
    for (i = 0; i < len; ++i)
    {
        char c = s[i];
        if (c == '|')
            s[i] = '\0';
    }
}
int go_dup(char *buff)
{
    char code[MAX_LEN];
    strcpy(code, buff);
    char *a = NULL;
    char *b = NULL;
    a = strchr(buff, '<');
    b = strchr(buff, '>');
    strip_dup(code);
    if (a != NULL && b != NULL)
    {
        char *in = a + 1 - buff + code;
        char *out = b + 1 - buff + code;
        strip_char(in, ' ');
        strip_char(out, ' ');
        fdin = open(in, O_RDWR, 0666);
        fdout = open(out, O_CREAT | O_RDWR, 0666);
        if (fdin == -1)
        {
            printf("File %s open faild\n", in);
            return -1;
        }
        if (fdout == -1)
        {
            printf("File %s open faild\n", out);
            return -1;
        }
        return mysys(code);
    }
    else if (a != NULL)
    {
        char *in = a + 1 - buff + code;
        strip_char(in, ' ');
        fdin = open(in, O_RDWR, 0666);
        fdout = RecoverPipeOut;
        if (fdin == -1)
        {
            printf("File %s open faild\n", in);
            return -1;
        }
        return mysys(code);
    }
    else if (b != NULL)
    {
        char *out = b + 1 - buff + code;
        strip_char(out, ' ');
        fdin = RecoverPipeIn;
        fdout = open(out, O_CREAT | O_RDWR, 0666);
        if (fdout == -1)
        {
            printf("File %s open faild\n", out);
            return -1;
        }
        return mysys(code);
    }
    else
    {
        fdin = RecoverPipeIn;
        fdout = RecoverPipeOut;
        return go(buff);
    }
}
void count_pipe(char *buff, int loc[])
{
    char *next = buff;
    loc[count_line++] = 0;
    while (next = strchr(next, '|'))
    {
        next = next + 1;
        loc[count_line++] = next - buff;
    }
}
int pipe_sys(const char *cmdstring)
{
    pid_t pid;
    pid = fork();
    if (pid == 0)
    {
        if (flag == 0)
        {
            dup2(fd[1], 1);
            close(fd[0]);
            close(fd[1]);
            execl("/bin/sh", "sh", "-c", cmdstring, (char *)0);
            exit(127);
        }
        else if (flag == 1)
        {
            dup2(fd[0], 0);
            close(fd[0]);
            close(fd[1]);
            execl("/bin/sh", "sh", "-c", cmdstring, (char *)0);
            exit(127);
        }
        else if (flag == 2)
        {
            dup2(fd[0], 0);
            close(fd[0]);
            close(fd[1]);
            // 输出进入临时管道 输出进入临时管道
            dup2(fd_temp[1], 1);
            close(fd_temp[0]);
            close(fd_temp[1]);
            execl("/bin/sh", "sh", "-c", cmdstring, (char *)0);
            exit(127);
        }
    }
    wait(NULL);
    return 0;
}
int my_pipe(char *buff)
{
    
    char code[MAX_LEN];
    strcpy(code, buff);
    strip_pipe(code);
    int position[MAX_NUM];
    int type;
    count_pipe(buff, position);
    if (count_line == 1)
    {
        fdin = RecoverPipeIn;
        fdout = RecoverPipeOut;
        return go_dup(buff);
    }
    for (int i = 0; i < count_line; i++)
    {
        if (flag == 2)
        {
            dup2(fd_temp[0], fd[0]);
            dup2(fd_temp[1], fd[1]);
            close(fd_temp[0]);
            close(fd_temp[1]);
            pipe(fd_temp);
            close(fd[1]);
        }
        if (flag == 0)
        {
            close(fd[1]);
        }
        if (i == 0)
        {
            flag = 0;
        }
        else if (i == count_line - 1)
        {
            flag = 1;
        }
        else
        {
            flag = 2;
        }
        type = pipe_sys(code + position[i]);
    }
    return type;
}
void find_last_dir(char **now)
{
    char *next = NULL;
    char *rest = dir;
    //printf("[dir] %s\n", dir);
    while (next = strchr(rest, '/'))
        rest = next + 1;
    if (rest == '\0')
        *now = dir;
    else
        *now = rest;
}
void print()
{
    if (strcmp(home, dir) == 0)
        printf(">");
    else
    {
        char *now = NULL;
        find_last_dir(&now);
        printf("> Please Input Command:");
    }
}
int main()
{
    pipe(fd);
    pipe(fd_temp);
    RecoverPipeIn = dup(0);
    RecoverPipeOut = dup(1);
    home = getenv("HOME");
    dir = getcwd(NULL, 0);
    char buff[MAX_LEN];
    print();
    while (fgets(buff, sizeof(buff), stdin))
    {
        strip(buff);
        my_pipe(buff);
        pipe(fd);
        pipe(fd_temp);
        print();
    }
    return 0;
}
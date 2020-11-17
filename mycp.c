#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
int CopyFile(int sour, int dest)
{
    char buf[64];
    int read_count;
    while (1)
    {
        read_count = read(sour, buf, sizeof(buf));
        if (read_count == 0)
        {
            break;
        }
        write(dest, buf, read_count);
    }
    return 0;
}
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("error:mycp need 2 parameters\n");
        return 0;
    }
    int sour, dest;
    sour = open(argv[1], O_RDONLY);
    if (sour == -1)
    {
        printf("error: can't find such file :%s\n", argv[1]);
        return 0;
    }
    dest = open(argv[2], O_CREAT | O_RDWR, 0777);
    if (dest == -1)
    {
        printf("error: can't creat such file :%s\n", argv[2]);
        return 0;
    }
    CopyFile(sour, dest);
    close(sour);
    close(dest);
    return 0;
}

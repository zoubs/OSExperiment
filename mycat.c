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
    char error1[30] = "error: mycat need a filename\n";
    char error2[40] = "error: mycat can't find such file\n";
    if (argc == 1)
    {
        write(1, error1, sizeof(error1));
    }
    int file;
    char *filepath;
    int i;
    for (i = 1; i < argc; i++)
    {
        filepath = argv[i];
        file = open(filepath, O_RDONLY);
        if (file == -1)
        {
            write(1, error2, sizeof(error2));
            continue;
        }
        printf("%s:\n",filepath);
        CopyFile(file, 1);
        close(file);
    }
    return 0;
}

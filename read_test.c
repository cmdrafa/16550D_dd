#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define buffersize 20

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("You must pass the device address as the first argument\n");
        return -1;
    }
    char *buf = malloc(buffersize + 2);
    char *dev = argv[1];
    char *q = "q\n";

    int fd = open(dev, O_RDWR);
    if (fd < 0)
    {
        perror("Opening device: ");
        return -1;
    }

    while (strcmp(buf, q) != 0)
    {
        int rc_w = read(fd, buf, 10);
        if (rc_w < 0)
        {
            perror("Reading FD: ");
            printf("%d\n", rc_w);
            return -1;
        }
        printf("RC_W: %s", buf);
    }

    fd = close(fd);

    free(buf);

    return 0;
}
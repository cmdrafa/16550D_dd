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

    int fd = open(dev, O_RDWR);
    if (fd < 0)
    {
        perror("Opening device: ");
        return -1;
    }

    while (*buf != 'q')
    {
        int rc_w = read(fd, buf, buffersize);
        if (rc_w < 0)
        {
            perror("Reading FD: ");
            printf("%d\n", rc_w);
            printf("errno = %d (expecting EAGAIN = %d)\n", errno, EAGAIN);
            if (errno == EAGAIN)
            {
                buf[0] = 0;
                continue;
            }
            else
                return -1;
        }
        if (strcmp(buf, "") != 0)
        {
            printf("Received String on userspace: %s\n", buf);
            //printf("Bytes read: %d \n", rc_w);
        }
    }

    fd = close(fd);

    free(buf);

    return 0;
}

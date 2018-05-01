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
    char *q = "quit\n";

    int fd = open(dev, O_RDWR);
    if (fd < 0)
    {
        perror("Opening device: ");
        return -1;
    }

    printf("Enter data to send to dd or 'quit' to quit: ");
    while (fgets(buf, buffersize + 2, stdin) != NULL)
    {
        int len = strlen(buf);
        //printf("LEN %d\n", len);
        int rc_w = write(fd, buf, len);
        if (strcmp(buf, q) == 0)
        {
            printf("Goodbye\n");
            break;
        }
        printf("Enter data to send to dd or 'quit' to quit: ");
    }

    fd = close(fd);

    free(buf);

    return 0;
}
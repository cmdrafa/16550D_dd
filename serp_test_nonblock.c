#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdbool.h>

#define buffersize 4096

int read_serp(fd)
{
    char *buf = malloc(buffersize + 2);
    char *q = "back\n";
    system("clear");
    fflush(stdin);

    printf("Send the string 'back' to get back to the menu!\n");
    while (1)
    {
        int rc_w = read(fd, buf, buffersize);
        if (rc_w < 0)
        {
            perror("Reading FD: ");
            return rc_w;
        }
        if (strcmp(buf, "") != 0)
        {
            printf("Received String: %s\n", buf);
            //printf("Bytes read: %d \n", rc_w);
        }
        if (strcmp(buf, q) == 0)
        {
            break;
        }
    }

    free(buf);

    return 0;
}

int write_serp(fd)
{
    char *buf = malloc(buffersize + 2);
    char *q = "back\n";
    system("clear");
    fflush(stdin);

    printf("Type the string to send or 'back' to go back to the menu!\n");
    while (1)
    {
        fgets(buf, buffersize + 2, stdin);
        int len = strlen(buf);
        int rc_w = write(fd, buf, len);
        if (rc_w < 0)
        {
            perror("error writing to the device: ");
            return -1;
        }
        if (strcmp(buf, q) == 0)
        {
            break;
        }
    }

    free(buf);

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Error!!\nYou must pass the device address as the second argument\n");
        return -1;
    }

    char *dev = argv[1];
    char option;
    bool isRunning = true;
    int rc;

    int fd = open(dev, O_RDWR | O_NONBLOCK);
    if (fd < 0)
    {
        perror("Opening device: ");
        return -1;
    }

    while (isRunning == true)
    {
        system("clear");
        fflush(stdin);
        puts("\n \t      COMMUNICATION WITH A UART DEVICE"
             "\n \t            FEUP - SO-2017/2018"
             "\n \t       SERP DEVICE DRIVER (O_NONBLOCK)"
             "\n***************************************************************"
             "\n[1]Read from the serial port"
             "\n[2]Write to the serial port"
             "\n[3]Exit"
             "\n***************************************************************");
        option = getchar();

        switch (option)
        {
        case '1':
            rc = read_serp(fd);
            if (rc < 0)
            {
                return rc;
            }
            break;
        case '2':
            write_serp(fd);
            break;
        case '3':
            isRunning = false;
            printf("Goodbye\n");
            break;
        }
    }

    close(fd);

    return 0;
}

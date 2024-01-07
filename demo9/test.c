#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define DEMO_DEV_NAME "/dev/demo_dev"

int main()
{
    int fd = open(DEMO_DEV_NAME, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "[demo] open device %s failed\n", DEMO_DEV_NAME);
        return -1;
    }
    close(fd);
    return 0;
}

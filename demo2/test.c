#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define DEMO_DEV_NAME "/dev/demo_dev"

int main()
{
    char buf[64];
    int fd = open(DEMO_DEV_NAME, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "[demo] open device %s failed\n", DEMO_DEV_NAME);
        return -1;
    }
    read(fd, buf, 64);
    close(fd);
    return 0;
}

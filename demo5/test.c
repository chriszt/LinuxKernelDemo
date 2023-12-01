#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define DEMO_DEV_NAME "/dev/my_demo_dev"

int main()
{
    int fd = open(DEMO_DEV_NAME, O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        fprintf(stderr, "[demo] open device %s failed\n", DEMO_DEV_NAME);
        return -1;
    }
    char msg[80] = "Testing the virtual FIFO device";
    size_t msgLen = sizeof(msg);
    char *readBuf = malloc(2 * msgLen);
    memset(readBuf, 0, 2 * msgLen);

    // 1. read
    int ret = read(fd, readBuf, 2 * msgLen);
    if (ret == -1) {
        fprintf(stderr, "[demo] read failed(%d): %s\n", errno, strerror(errno));
    } else {
        fprintf(stdout, "[demo] read %d bytes, readBuf=%s\n", ret, readBuf);
    }

    // 2. write
    ret = write(fd, msg, msgLen);
    if (ret == -1) {
        fprintf(stderr, "[demo] write failed(%d): %s\n", errno, strerror(errno));
    } else {
        fprintf(stderr, "[demo] write %d bytes\n", ret);
    }

    // 3. write
    ret = write(fd, msg, msgLen);
    if (ret == -1) {
        fprintf(stderr, "[demo] write failed(%d): %s\n", errno, strerror(errno));
    } else {
        fprintf(stderr, "[demo] write %d bytes\n", ret);
    }

    // 4. read
    ret = read(fd, readBuf, 2 * msgLen);
    if (ret == -1) {
        fprintf(stderr, "[demo] read failed(%d): %s\n", errno, strerror(errno));
    } else {
        fprintf(stdout, "[demo] read %d bytes, readBuf=%s\n", ret, readBuf);
    }

    close(fd);
    return 0;
}

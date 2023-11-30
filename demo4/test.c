#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define DEMO_DEV_NAME "/dev/my_demo_dev"

int main()
{
    int fd = open(DEMO_DEV_NAME, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "[demo] open device %s failed\n", DEMO_DEV_NAME);
        return -1;
    }
    // char msg[] = "Testing the virtual FIFO device";
    char msg[] = "abc";
    size_t msgLen = sizeof(msg);
    int ret = write(fd, msg, msgLen);
    if (ret != msgLen) {
        fprintf(stderr, "[demo] write to device(%d) failed: ret=%d, errno=%d, %s\n", fd, ret, errno, strerror(errno));
        close(fd);
        return -1;
    }
    fprintf(stdout, "[demo] write %d bytes to device\n", ret);
    close(fd);

    fd = open(DEMO_DEV_NAME, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "[demo] open device %s failed\n", DEMO_DEV_NAME);
        return -1;
    }
    char buf[10] = {};
    ret = read(fd, buf, sizeof(buf));
    fprintf(stdout, "[demo] read %d bytes, buf=%s\n", ret, buf);
    close(fd);
    return 0;
}

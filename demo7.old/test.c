#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <poll.h>

int main()
{
    struct pollfd fds[2];
    fds[0].fd = open("/dev/my_demo_dev0", O_RDWR);
    fds[0].events = POLLIN;
    fds[1].fd = open("/dev/my_demo_dev1", O_RDWR);
    fds[1].events = POLLIN;

    char buf0[64];
    char buf1[64];

    while (1) {
        int ret = poll(fds, 2, -1);
        if (ret == -1) {
            return ret;
        }
        if (fds[0].revents & POLLIN) {
            memset(buf0, 0, sizeof(buf0));
            ret = read(fds[0].fd, buf0, sizeof(buf0));
            if (ret < 0) {
                return ret;
            }
            fprintf(stdout, "[demo] buf0=%s\n", buf0);
        }
        if (fds[1].revents & POLLIN) {
            memset(buf1, 0, sizeof(buf1));
            ret = read(fds[1].fd, buf1, sizeof(buf1));
            if (ret < 0) {
                return ret;
            }
            fprintf(stdout, "[demo] buf1=%s\n", buf1);
        }
    }

    return 0;
}

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main()
{
    fprintf(stdout, "%s\n", strerror(2));

    return 0;
}

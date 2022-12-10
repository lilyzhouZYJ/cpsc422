#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <file.h>

#define exit(...) return __VA_ARGS__

int main(int argc, char **argv)
{
    printf("ping started.\n");

    int fd = open("hello_world", O_CREATE | O_RDONLY);

    /* acquire shared lock */
    if (flock(fd, LOCK_SH) == -1) {
        printf("failed to acquire shared lock\n");
        exit(1);
    }
    /* acquire shared lock in non-blocking mode */
    if (flock(fd, LOCK_SH | LOCK_NB) == -1) {
        exit(1);
    }
    /* non-atomically upgrade to exclusive lock */
    if (flock(fd, LOCK_EX) == -1) {
        exit(1);
    }
    /* release lock */
    /* lock is also released automatically when close() is called or process exits */
    if (flock(fd, LOCK_UN) == -1) {
        exit(1);
    }

    printf("ping terminated\n");

    return 0;
}

#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <file.h>

#define exit(...) return __VA_ARGS__

int main(int argc, char **argv)
{
    printf("flock1: flock1 started.\n\n");

    int fd1 = open("flock_file", O_CREATE | O_RDWR);

    // (1) fd1 acquire exclusive lock
    if(flock(fd1, LOCK_EX) < 0){
        printf("flock1: fd1 fail to acquire exclusive lock\n");
    } else {
        printf("flock1: fd1 acquired exclusive lock\n");
    }

    // (2) fd1 acquire shared lock
    if(flock(fd1, LOCK_SH) < 0){
        printf("flock1: fd1 fail to acquire shared lock\n");
    } else {
        printf("flock1: fd1 acquired shared lock\n");
    }

    // (3) fd1 release lock
    if(flock(fd1, LOCK_UN) < 0){
        printf("flock1: fd1 fail to release lock\n");
    } else {
        printf("flock1: fd1 released lock\n");
    }

    // (4) fd1 acquire shared lock
    if(flock(fd1, LOCK_SH) < 0){
        printf("flock1: fd1 fail to acquire shared lock\n");
    } else {
        printf("flock1: fd1 acquired shared lock\n");
    }

    printf("flock1: flock1 terminated\n");

    return 0;
}

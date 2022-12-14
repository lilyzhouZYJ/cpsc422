#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <file.h>

#define exit(...) return __VA_ARGS__

int main(int argc, char **argv)
{
    printf("flock2: flock2 started.\n");

    int fd1 = open("README", O_RDWR);

    // (1) fd1 acquire exclusive lock
    if(flock(fd1, LOCK_EX) < 0){
        printf("flock2: fd1 fail to acquire exclusive lock\n");
    } else {
        printf("flock2: fd1 acquired exclusive lock\n");
    }

    char buf[10];
    read(fd1, buf, 6);

    yield();

    // (2) fd1 acquire shared lock
    if(flock(fd1, LOCK_SH) < 0){
        printf("flock2: fd1 fail to acquire shared lock\n");
    } else {
        printf("flock2: fd1 acquired shared lock\n");
    }

    yield();

    // (3) fd1 release lock
    if(flock(fd1, LOCK_UN) < 0){
        printf("flock2: fd1 fail to release shared lock\n");
    } else {
        printf("flock2: fd1 released shared lock\n");
    }

    yield();

    // (4) fd1 acquire shared lock
    if(flock(fd1, LOCK_SH) < 0){
        printf("flock2: fd1 fail to acquire shared lock\n");
    } else {
        printf("flock2: fd1 acquired shared lock\n");
    }

    // (5) fd1 release shared lock
    if(flock(fd1, LOCK_UN) < 0){
        printf("flock2: fd1 fail to release shared lock\n");
    } else {
        printf("flock2: fd1 released shared lock\n");
    }

    printf("flock2: flock2 terminated\n");

    return 0;
}

#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <file.h>

#define exit(...) return __VA_ARGS__

int main(int argc, char **argv)
{
    printf("ping started.\n");

    int fd1 = open("file1", O_CREATE | O_RDONLY);
    int fd2 = open("file1", O_RDWR);

    // Acquire exclusive locks
    if(flock(fd1, LOCK_EX) < 0){
        printf("could not acquire exclusive lock on fd1\n");
    }
    if(flock(fd2, LOCK_EX) < 0){
        printf("could not acquire exclusive lock on fd2");
    }

    // Convert both to shared lock
    if(flock(fd1, LOCK_SH) < 0){
        printf("could not convert fd1 lock from exclusive to shared\n");
    }
    if(flock(fd2, LOCK_SH) < 0){
        printf("could not convert fd2 lock from exclusive to shared\n");
    }

    // Convert fd1 back to exclusive
    if(flock(fd1, LOCK_EX) < 0){
        printf("could not convert fd1 lock from shared to exclusive\n");
    }

    printf("ping terminated\n");

    return 0;
}

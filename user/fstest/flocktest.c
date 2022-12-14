#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <file.h>

#define exit(...) return __VA_ARGS__

void test_shared()
{
    // Two independent fd's
    int fd1 = open("shared", O_CREATE);
    int fd2 = open("shared", O_RDWR);

    // (1) fd1 acquire shared lock => expect success
    if(flock(fd1, LOCK_SH) < 0){
        printf("fd1 cannot acquire shared lock\n");
    } else {
        printf("fd1 now holds shared lock\n");
    }

    // (2) fd2 also acquire shared lock => expect success
    if(flock(fd2, LOCK_SH) < 0){
        printf("fd2 cannot acquire shared lock\n");
    } else {
        printf("fd2 now holds shared lock\n");
    }

    // (3) fd1 try to convert to exclusive => expect failure
    if(flock(fd1, LOCK_EX | LOCK_NB) < 0){
        printf("fd1 cannot convert to exclusive lock\n");
    } else {
        printf("fd1 has converted to exclusive lock\n");
    }

    // (4) fd2 release shared lock => expect success
    if(flock(fd2, LOCK_UN) < 0){
        printf("fd2 cannot release lock\n");
    } else {
        printf("fd2 has released lock\n");
    }

    // (4) fd1 convert to exclusive => expect success
    if(flock(fd1, LOCK_EX | LOCK_NB) < 0){
        printf("fd1 cannot convert to exclusive lock\n");
    } else {
        printf("fd1 has converted to exclusive lock\n");
    }
}

void test_exclusive()
{
    // Two independent fd's
    int fd1 = open("file1", O_CREATE);
    int fd2 = open("file1", O_RDWR);

    // (1) fd1 acquire exclusive lock => expect success
    if(flock(fd1, LOCK_EX) < 0){
        printf("fd1 cannot acquire exclusive lock\n");
    } else {
        printf("fd1 now holds exclusive lock\n");
    }

    // (2) fd2 acquire exclusive lock in blocking mode => will freeze
    // if(flock(fd2, LOCK_EX) < 0){
    //     printf("could not acquire exclusive lock on fd2");
    // }

    // (3) fd2 acquire exclusive lock in non-blocking mode => expect failure
    if(flock(fd2, LOCK_EX | LOCK_NB) < 0){
        printf("fd2 cannot acquire exclusive lock\n");
    } else {
        printf("fd2 now holds exclusive lock\n");
    }

    // (4) fd1 release lock => expect success
    if(flock(fd1, LOCK_UN) < 0){
        printf("fd1 cannot release lock\n");
    } else {
        printf("fd1 released lock\n");
    }

    // (5) fd2 acquire exclusive lock => expect success
    if(flock(fd2, LOCK_EX) < 0){
        printf("fd2 cannot acquire exclusive lock\n");
    } else {
        printf("fd2 now holds exclusive lock\n");
    }
}

void test_release_lock()
{
    // Two independent fd's
    int fd1 = open("release_lock", O_CREATE);
    int fd2 = open("release_lock", O_RDWR);

    // (1) fd1 acquire exclusive lock => expect success
    if(flock(fd1, LOCK_EX) < 0){
        printf("fd1 cannot acquire exclusive lock\n");
    } else {
        printf("fd1 now holds exclusive lock\n");
    }

    // (2) fd2 release lock => expect failure
    if(flock(fd2, LOCK_UN) < 0){
        printf("fd2 cannot release lock\n");
    } else {
        printf("fd2 has released lock\n");
    }

    // (3) fd1 release lock => expect success
    if(flock(fd1, LOCK_UN) < 0){
        printf("fd1 cannot release lock\n");
    } else {
        printf("fd1 has released lock\n");
    }

    // (4) fd2 acquire shared lock => expect success
    if(flock(fd2, LOCK_SH) < 0){
        printf("fd2 cannot acquire shared lock\n");
    } else {
        printf("fd2 now holds shared lock\n");
    }

    // (5) fd1 release lock => expect failure
    if(flock(fd1, LOCK_UN) < 0){
        printf("fd1 cannot release lock\n");
    } else {
        printf("fd1 has released lock\n");
    }
}

void test_close_file()
{
    int fd1 = open("close_file", O_CREATE);
    int fd2 = open("close_file", O_RDWR);

    // (1) fd1 acquire shared lock => expect success
    if(flock(fd1, LOCK_SH) < 0){
        printf("fd1 cannot acquire shared lock\n");
    } else {
        printf("fd1 now holds shared lock\n");
    }

    // (2) fd2 acquire shared lock => expect success
    if(flock(fd2, LOCK_SH) < 0){
        printf("fd2 cannot acquire shared lock\n");
    } else {
        printf("fd2 now holds shared lock\n");
    }

    // (3) fd2 try to convert to exclusive lock => expect failure
    if(flock(fd2, LOCK_EX | LOCK_NB) < 0){
        printf("fd2 cannot convert to exclusive lock\n");
    } else {
        printf("fd2 has been converted to exclusive lock\n");
    }

    // (3) Close fd1
    printf("closing fd1\n");
    close(fd1);

    // (4) fd2 convert to exclusive lock => expect success
    if(flock(fd2, LOCK_EX) < 0){
        printf("fd2 cannot convert to exclusive lock\n");
    } else {
        printf("fd2 has been converted to exclusive lock\n");
    }
}

void test_multithread()
{
    pid_t flock1_pid;
    if ((flock1_pid = spawn(8, 500)) != -1)
        printf("flock1 in process %d.\n", flock1_pid);
    else
        printf("Failed to launch flock1.\n");

    pid_t flock2_pid;
    if ((flock2_pid = spawn(9, 500)) != -1)
        printf("flock2 in process %d.\n", flock2_pid);
    else
        printf("Failed to launch flock2.\n");
    
    pid_t flock3_pid;
    if ((flock3_pid = spawn(10, 500)) != -1)
        printf("flock3 in process %d.\n", flock3_pid);
    else
        printf("Failed to launch flock3.\n");
}

int main(int argc, char **argv)
{
    printf("flocktest started.\n\n");
    
    printf("======== test_shared ========\n");
    test_shared();
    printf("======== test_shared ends ========\n\n");

    printf("======== test_exclusive ========\n");
    test_exclusive();
    printf("======== test_exclusive ends ========\n\n");

    printf("======== test_release_lock ========\n");
    test_release_lock();
    printf("======== test_release_lock ends ========\n\n");

    printf("======== test_close_file ========\n");
    test_close_file();
    printf("======== test_close_file ends ========\n\n");

    printf("======== test_multithread ========\n");
    test_multithread();

    return 0;
}

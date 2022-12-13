#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <x86.h>

int main(int argc, char **argv)
{
    printf("idle\n");

#ifdef TEST
    pid_t fstest_pid;
    if ((fstest_pid = spawn(4, 1000)) != -1)
        printf("fstest in process %d.\n", fstest_pid);
    else
        printf("Failed to launch fstest.\n");
#else
    // pid_t shell_pid;
    // if ((shell_pid = spawn(5, 1000)) != -1)
    //     printf("shell in process %d.\n", shell_pid);
    // else
    //     printf("Failed to launch shell.\n");
    
    pid_t flock_pid;
    if ((flock_pid = spawn(7, 2000)) != -1)
        printf("flocktest in process %d.\n", flock_pid);
    else
        printf("Failed to launch flocktest.\n");

    // pid_t fork_pid;
    // if ((fork_pid = spawn(6, 2000)) != -1)
    //     printf("fork_test in process %d.\n", fork_pid);
    // else
    //     printf("Failed to launch fork_test.\n");
#endif

    return 0;
}

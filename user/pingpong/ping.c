#include <proc.h>
#include <stdio.h>
#include <syscall.h>

int main(int argc, char **argv)
{
    unsigned int i;
    printf("ping started.\n");

    // fast producing
    // for (i = 0; i < 10; i++)
    //     produce();

    // // slow producing
    // for (i = 0; i < 40; i++) {
    //     if (i % 4 == 0)
    //         produce();
    // }

    for (i = 0; i < 200; i++){
        produce();
    }

    printf("ping ended.\n");

    return 0;
}

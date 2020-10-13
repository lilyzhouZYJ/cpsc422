#include <proc.h>
#include <stdio.h>
#include <syscall.h>

int main(int argc, char **argv)
{
    unsigned int i;
    printf("ping started.\n");

    // fast producing
    for (i = 0; i < 10; i++)
        produce(i);

    // slow producing
    for (i = 0; i < 40; i++) {
        if (i % 4 == 0)
            produce(10 * i);
    }

    return 0;
}

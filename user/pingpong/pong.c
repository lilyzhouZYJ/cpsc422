#include <proc.h>
#include <stdio.h>
#include <syscall.h>

int main(int argc, char **argv)
{
    unsigned int i;
    printf("pong started.\n");

    // for (i = 0; i < 20; i++) {
    //     if (i % 2 == 0)
    //         consume();
    // }

    for(i = 0; i < 200; i++){
        consume();
        for(int j = 0; j < 2000; j++){ }
    }

    printf("pong ended.\n");

    return 0;
}

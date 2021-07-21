#include "calculator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_help(void) {
    printf("calculator \"[expression]\"\n");
    printf("example: `calculator \"1+2*(7.5*2)\"`\n");
}

int main(int argc, char **argv) {
    --argc; ++argv;
    if (argc == 0) {
        print_help();
        return 0;
    } else if (argc > 1) {
        fprintf(stderr, "too many parameters to the program!\n");
        return 1;
    }
    printf("%f\n", calculate(argv[0]));
    return 0;
}


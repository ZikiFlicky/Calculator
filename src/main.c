#include "calculator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BUFFER_SIZE 128

static char *readline(char *prompt) {
    char *value = NULL;
    char buffer[BUFFER_SIZE+1];
    size_t i;
    char c;
    bool read = true;
    size_t allocated = 0;

    if (prompt) printf("%s", prompt);

    while (read) {
        for (i = 0; i < BUFFER_SIZE + 1; ++i)
            buffer[i] = '\0';
        for (i = 0; i < BUFFER_SIZE; ++i) {
            c = fgetc(stdin);
            // matches \r\n, \r, and \n line endings
            if (c == '\r' || c == '\n') {
                read = false;
                break;
            }
            buffer[i] = c;
        }
        buffer[i] = '\0';
        allocated += i;
        if (!value)
            value = malloc((allocated+1)*sizeof(char));
        else
            value = realloc(value, (allocated+1)*sizeof(char));
        strcpy(&value[allocated-i], buffer);
    }
    return value;
}

static void repl(void) {
    char *string;
    CalculatorNumber result;

    for (;;) {
        string = readline("calculator> ");
        if (!string)
            continue;
        if (strcmp(string, "exit") == 0) {
            free(string);
            break;
        }
        result = calculate(string);
        free(string);
        print_calculator_number(&result);
        printf("\n");
    }
}

static void print_help(void) {
    printf("usage: calculator [option [expression]]\n");
    printf("  -h  --help         print this message and exit\n");
    printf("  -i  --interactive  start repl\n");
    printf("  -c  --calculate    calculate string\n");
    printf("example:\n");
    printf("  calculator -c \"1+2*(7.5*2)\"\n");
}

int main(int argc, char **argv) {
    CalculatorNumber result;
    --argc; ++argv;
    if (argc == 0) {
        print_help();
        return 0;
    }
    if (strcmp(argv[0], "-h") == 0 || strcmp(argv[0], "--help") == 0) {
        print_help();
        return 0;
    }
    if (strcmp(argv[0], "-i") == 0 || strcmp(argv[0], "--interactive") == 0) {
        if (argc > 1) {
            print_help();
            return 1;
        }
        repl();
        return 0;
    }
    if (strcmp(argv[0], "-c") == 0 || strcmp(argv[0], "--calculate") == 0) {
        if (argc < 2) {
            print_help();
            return 1;
        }
        if (argc > 2) {
            print_help();
            return 1;
        }
        result = calculate(argv[1]);
        print_calculator_number(&result);
        printf("\n");
        return 0;
    }
    print_help();
    return 1;
}

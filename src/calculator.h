#ifndef CALCULATOR_CALCULATOR_H
#define CALCULATOR_CALCULATOR_H

#include <stdbool.h>

typedef struct {
    bool is_int;
    int as_int;
    double as_float;
} CalculatorNumber;

CalculatorNumber calculate(char *stream);

#endif /* CALCULATOR_CALCULATOR_H */

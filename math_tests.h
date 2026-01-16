#ifndef MATH_TESTS_H
#define MATH_TESTS_H

#include <stdint.h>

typedef enum {
    MATH_TEST_FOR,
    MATH_TEST_WHILE,
    MATH_TEST_VERA_UNTIL0,
}_eMathTest;

void MathTestsinit();
void MathTest(_eMathTest t);




#endif //MATH_TESTS_H
#ifndef MATH_TESTS_H
#define MATH_TESTS_H

#include <stdint.h>

typedef enum {
    MATH_TEST_NOTHING,
    MATH_TEST_FOR,
    MATH_TEST_WHILE,
    MATH_TEST_VERA_UNTIL0,
    MATH_TEST_VERA_REPEAT_MACRO,
    MATH_TEST_VERA_UNTIL0_ASM,
    MATH_TEST_WASTE_TIME,



    MATH_TEST_COUNT,
}_eMathTest;

void MathTestsinit();
void MathTest(_eMathTest t);




#endif //MATH_TESTS_H
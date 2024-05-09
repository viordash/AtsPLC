#include "CppUTest/TestHarness.h"

#include <stdarg.h>
#include <stdio.h>

TEST_GROUP(Ya_Task1_TestsGroup){};

//1. Средний элемент

int med(int a, int b, int c) {

    int ab = a > b;
    int bc = b > c;

    int abc = ab && bc;
    if (abc) {
        return b;
    }

    int ac = a > c;
    int acb = ac && !bc;
    if (acb) {
        return c;
    }

    int cba = !bc && !ab;
    if (cba) {
        return b;
    }

    int bca = bc && !ac;
    if (bca) {
        return c;
    }

    // int cab = !ac && ab;
    // if(cab) {
    return a;
    //}

    // int bac = !ab && ac;
    // if(bac) {
    return a;
    //}
    return INT_MIN;
}

TEST(Ya_Task1_TestsGroup, find_middle) {
    CHECK_EQUAL(med(3, 2, 1), 2);
    CHECK_EQUAL(med(1, 2, 3), 2);
    CHECK_EQUAL(med(1, 1, 1), 1);

    CHECK_EQUAL(med(-1000, 1000, 0), 0);
    CHECK_EQUAL(med(3, 3, 1), 3);
    CHECK_EQUAL(med(3, 3, 2), 3);
    CHECK_EQUAL(med(2, 3, 2), 2);
}
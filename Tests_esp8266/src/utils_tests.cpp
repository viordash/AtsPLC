#include "CppUTest/TestHarness.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "main/utils.cpp"

TEST_GROUP(UtilsTestsGroup){ //
                             TEST_SETUP(){}

                             TEST_TEARDOWN(){}
};

TEST(UtilsTestsGroup, hexchar_to_dec) {
    CHECK_EQUAL(0, hexchar_to_dec('0'));
    CHECK_EQUAL(1, hexchar_to_dec('1'));
    CHECK_EQUAL(2, hexchar_to_dec('2'));
    CHECK_EQUAL(3, hexchar_to_dec('3'));
    CHECK_EQUAL(4, hexchar_to_dec('4'));
    CHECK_EQUAL(5, hexchar_to_dec('5'));
    CHECK_EQUAL(6, hexchar_to_dec('6'));
    CHECK_EQUAL(7, hexchar_to_dec('7'));
    CHECK_EQUAL(8, hexchar_to_dec('8'));
    CHECK_EQUAL(9, hexchar_to_dec('9'));
    CHECK_EQUAL(10, hexchar_to_dec('a'));
    CHECK_EQUAL(10, hexchar_to_dec('A'));
    CHECK_EQUAL(11, hexchar_to_dec('b'));
    CHECK_EQUAL(11, hexchar_to_dec('B'));
    CHECK_EQUAL(12, hexchar_to_dec('c'));
    CHECK_EQUAL(12, hexchar_to_dec('C'));
    CHECK_EQUAL(13, hexchar_to_dec('d'));
    CHECK_EQUAL(13, hexchar_to_dec('D'));
    CHECK_EQUAL(14, hexchar_to_dec('e'));
    CHECK_EQUAL(14, hexchar_to_dec('E'));
    CHECK_EQUAL(15, hexchar_to_dec('f'));
    CHECK_EQUAL(15, hexchar_to_dec('F'));
}

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/ElementsBox.cpp"
#include "main/LogicProgram/Inputs/InputNC.h"

TEST_GROUP(LogicElementsBoxTestsGroup){ //
                                        TEST_SETUP(){ mock().disable(); }

                                        TEST_TEARDOWN(){ mock().enable();
}
}
;

TEST(LogicElementsBoxTestsGroup, box_for_inputs_elements) {
    InputNC stored_element;
    ElementsBox testable(100, &stored_element);
    CHECK_EQUAL(5, testable.size());
}

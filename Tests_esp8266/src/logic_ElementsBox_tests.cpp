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
#include "main/LogicProgram/Outputs/IncOutput.h"

TEST_GROUP(LogicElementsBoxTestsGroup){ //
                                        TEST_SETUP(){ mock().disable(); }

                                        TEST_TEARDOWN(){ mock().enable();
}
}
;

TEST(LogicElementsBoxTestsGroup, box_for_inputs_elements) {
    InputNC stored_element(MapIO::V1);
    ElementsBox testable(100, &stored_element);
    CHECK_EQUAL(8, testable.size());
}

TEST(LogicElementsBoxTestsGroup, box_for_outputs_elements) {
    IncOutput stored_element(MapIO::O1);
    ElementsBox testable(100, &stored_element);
    CHECK_EQUAL(4, testable.size());
}

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
                                        TEST_SETUP(){ mock().disable();
}

TEST_TEARDOWN() {
    mock().enable();
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

TEST(LogicElementsBoxTestsGroup, takes_params_from_stored_input_element) {
    InputNC stored_element(MapIO::V1);
    ElementsBox testable(100, &stored_element);
    CHECK_EQUAL(8, testable.size());
    for (auto *element : testable) {
        auto *element_as_commonInput = CommonInput::TryToCast(element);
        if (element_as_commonInput != NULL) {
            CHECK_EQUAL(MapIO::V1, element_as_commonInput->GetIoAdr());
        }
    }
}

TEST(LogicElementsBoxTestsGroup, takes_params_from_stored_output_element) {
    IncOutput stored_element(MapIO::O1);
    ElementsBox testable(100, &stored_element);
    CHECK_EQUAL(4, testable.size());
    for (auto *element : testable) {
        auto *element_as_commonOutput = CommonOutput::TryToCast(element);
        if (element_as_commonOutput != NULL) {
            CHECK_EQUAL(MapIO::O1, element_as_commonOutput->GetIoAdr());
        }
    }
}

TEST(LogicElementsBoxTestsGroup, no_available_place_for_timers_and_comparators) {
    InputNC stored_element(MapIO::V1);
    ElementsBox testable(30, &stored_element);
    CHECK_EQUAL(1, testable.size());
}
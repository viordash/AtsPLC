#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/ControllerVariable.h"
#include "main/LogicProgram/LogicElement.h"

TEST_GROUP(LogicControllerVariableTestsGroup){ //
                                               TEST_SETUP(){}

                                               TEST_TEARDOWN(){}
};

namespace {
    class TestableControllerVariable : public ControllerVariable {
      public:
        bool *PublicMorozov_Get_required() {
            return &required;
        }
        uint8_t *PublicMorozov_Get_out_value() {
            return &out_value;
        }
    };
} // namespace

TEST(LogicControllerVariableTestsGroup, Init_reset_value_and_set_requried) {
    TestableControllerVariable testable;
    *(testable.PublicMorozov_Get_required()) = false;
    testable.UpdateValue(LogicElement::MaxValue);

    testable.Init();
    CHECK_TRUE(*(testable.PublicMorozov_Get_required()));
    CHECK_EQUAL(LogicElement::MinValue, testable.PeekValue());
}

TEST(LogicControllerVariableTestsGroup, SampleValue_reset_requried) {
    TestableControllerVariable testable;
    testable.Init();

    CHECK_TRUE(*(testable.PublicMorozov_Get_required()));
    testable.SampleValue();
    CHECK_FALSE(*(testable.PublicMorozov_Get_required()));
}

TEST(LogicControllerVariableTestsGroup, SampleValue_return_true_if_any_changes) {
    TestableControllerVariable testable;
    testable.Init();

    testable.WriteValue(42);
    CHECK_TRUE(testable.SampleValue());
    CHECK_EQUAL(42, testable.ReadValue());
    CHECK_FALSE(testable.SampleValue());
}

TEST(LogicControllerVariableTestsGroup, UpdateValue_return_true_if_any_changes) {
    TestableControllerVariable testable;
    testable.Init();

    CHECK_TRUE(testable.UpdateValue(LogicElement::MaxValue));
    CHECK_FALSE(testable.UpdateValue(LogicElement::MaxValue));
}

TEST(LogicControllerVariableTestsGroup, UpdateValue_updated_value) {
    TestableControllerVariable testable;
    testable.Init();

    CHECK_TRUE(testable.UpdateValue(LogicElement::MaxValue));
    CHECK_EQUAL(LogicElement::MaxValue, testable.PeekValue());
}

TEST(LogicControllerVariableTestsGroup, ReadValue_returns_value_and_set_requried) {
    TestableControllerVariable testable;
    testable.Init();
    *(testable.PublicMorozov_Get_required()) = false;
    CHECK_TRUE(testable.UpdateValue(LogicElement::MaxValue));

    CHECK_EQUAL(LogicElement::MaxValue, testable.ReadValue());
    CHECK_TRUE(*(testable.PublicMorozov_Get_required()));
}

TEST(LogicControllerVariableTestsGroup, WriteValue_changes_out_value_and_set_requried) {
    TestableControllerVariable testable;

    testable.WriteValue(LogicElement::MaxValue);
    CHECK_EQUAL(LogicElement::MaxValue, *testable.PublicMorozov_Get_out_value());

    testable.WriteValue(42);
    CHECK_EQUAL(42, *testable.PublicMorozov_Get_out_value());
    CHECK_TRUE(*(testable.PublicMorozov_Get_required()));
}

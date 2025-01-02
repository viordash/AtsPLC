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
        bool *PublicMorozov_Get_required_reading() {
            return &required_reading;
        }
        bool *PublicMorozov_Get_required_writing() {
            return &required_writing;
        }
        uint8_t *PublicMorozov_Get_out_value() {
            return &out_value;
        }
        uint8_t *PublicMorozov_Get_value() {
            return &value;
        }
    };
} // namespace

TEST(LogicControllerVariableTestsGroup, Init_reset_value_and_set_required) {
    TestableControllerVariable testable;
    *(testable.PublicMorozov_Get_required_reading()) = false;
    testable.UpdateValue(LogicElement::MaxValue);

    testable.Init();
    CHECK_TRUE(*(testable.PublicMorozov_Get_required_reading()));
    CHECK_FALSE(*(testable.PublicMorozov_Get_required_writing()));
    CHECK_EQUAL(LogicElement::MinValue, testable.PeekValue());
    CHECK_EQUAL(LogicElement::MinValue, *testable.PublicMorozov_Get_out_value());
}

TEST(LogicControllerVariableTestsGroup, SampleValue_reset_required_reading) {
    TestableControllerVariable testable;
    testable.Init();

    CHECK_TRUE(*(testable.PublicMorozov_Get_required_reading()));
    testable.SampleValue();
    CHECK_FALSE(*(testable.PublicMorozov_Get_required_reading()));
}

TEST(LogicControllerVariableTestsGroup, SampleValue_return_true_if_any_changes) {
    TestableControllerVariable testable;
    testable.Init();

    *testable.PublicMorozov_Get_value() = 42;
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

TEST(LogicControllerVariableTestsGroup, ReadValue_returns_value) {
    TestableControllerVariable testable;
    testable.Init();
    CHECK_TRUE(testable.UpdateValue(LogicElement::MaxValue));

    CHECK_EQUAL(LogicElement::MaxValue, testable.ReadValue());
}

TEST(LogicControllerVariableTestsGroup,
     ReadValue_set_required_reading_and_not_touch_required_writing) {
    TestableControllerVariable testable;
    testable.Init();
    *(testable.PublicMorozov_Get_required_reading()) = false;

    testable.ReadValue();
    CHECK_TRUE(*(testable.PublicMorozov_Get_required_reading()));
    CHECK_FALSE(*(testable.PublicMorozov_Get_required_writing()));
}

TEST(LogicControllerVariableTestsGroup, WriteValue_changes_out_value) {
    TestableControllerVariable testable;

    testable.WriteValue(LogicElement::MaxValue);
    CHECK_EQUAL(LogicElement::MaxValue, *testable.PublicMorozov_Get_out_value());

    testable.WriteValue(42);
    CHECK_EQUAL(42, *testable.PublicMorozov_Get_out_value());
    CHECK_TRUE(*(testable.PublicMorozov_Get_required_writing()));
}

TEST(LogicControllerVariableTestsGroup,
     WriteValue_set_required_writing_and_not_touch_required_reading) {
    TestableControllerVariable testable;
    testable.Init();
    *(testable.PublicMorozov_Get_required_reading()) = false;

    testable.WriteValue(42);
    CHECK_TRUE(*(testable.PublicMorozov_Get_required_writing()));
    CHECK_FALSE(*(testable.PublicMorozov_Get_required_reading()));
}

TEST(LogicControllerVariableTestsGroup, CommitChanges_reset_required_writing) {
    TestableControllerVariable testable;
    testable.Init();
    *(testable.PublicMorozov_Get_required_writing()) = true;

    testable.CommitChanges();
    CHECK_FALSE(*(testable.PublicMorozov_Get_required_writing()));
}

TEST(LogicControllerVariableTestsGroup, CommitChanges_updated_value) {
    TestableControllerVariable testable;
    testable.Init();
    testable.WriteValue(42);

    testable.CommitChanges();
    CHECK_EQUAL(42, *testable.PublicMorozov_Get_value());
}

TEST(LogicControllerVariableTestsGroup, Value_changes_in_transaction) {
    TestableControllerVariable testable;
    testable.Init();

    CHECK_FALSE(testable.SampleValue());
    CHECK_EQUAL(LogicElement::MinValue, testable.ReadValue());

    testable.WriteValue(42);
    CHECK_EQUAL(LogicElement::MinValue, testable.ReadValue());

    testable.CommitChanges();
    CHECK_EQUAL(LogicElement::MinValue, testable.ReadValue());

    CHECK_TRUE(testable.SampleValue());
    CHECK_EQUAL(42, testable.ReadValue());
}
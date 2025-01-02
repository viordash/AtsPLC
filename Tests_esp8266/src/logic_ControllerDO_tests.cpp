#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/ControllerDO.h"
#include "main/LogicProgram/LogicElement.h"
#include "sys_gpio.h"

TEST_GROUP(LogicControllerDOTestsGroup){ //
                                         TEST_SETUP(){}

                                         TEST_TEARDOWN(){}
};

namespace {
    class TestableControllerDO : public ControllerDO {
      public:
        TestableControllerDO(gpio_output gpio) : ControllerDO(gpio) {
        }
        bool *PublicMorozov_Get_required_reading() {
            return &required_reading;
        }
        bool *PublicMorozov_Get_required_writing() {
            return &required_writing;
        }
        uint8_t *PublicMorozov_Get_out_value() {
            return &out_value;
        }
    };
} // namespace

TEST(LogicControllerDOTestsGroup, Init_reset_value_and_set_required) {
    TestableControllerDO testable(gpio_output::OUTPUT_0);
    *(testable.PublicMorozov_Get_required_reading()) = false;
    testable.UpdateValue(LogicElement::MaxValue);

    testable.Init();
    CHECK_TRUE(*(testable.PublicMorozov_Get_required_reading()));
    CHECK_FALSE(*(testable.PublicMorozov_Get_required_writing()));
    CHECK_EQUAL(LogicElement::MinValue, testable.PeekValue());
    CHECK_EQUAL(LogicElement::MinValue, *testable.PublicMorozov_Get_out_value());
}

TEST(LogicControllerDOTestsGroup, SampleValue_reset_required_reading) {
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    TestableControllerDO testable(gpio_output::OUTPUT_0);
    testable.Init();

    CHECK_TRUE(*(testable.PublicMorozov_Get_required_reading()));
    testable.SampleValue();
    CHECK_FALSE(*(testable.PublicMorozov_Get_required_reading()));
}

TEST(LogicControllerDOTestsGroup, SampleValue_return_true_if_any_changes) {
    mock("2").expectNCalls(2, "gpio_get_level").andReturnValue(0);
    TestableControllerDO testable(gpio_output::OUTPUT_0);
    testable.Init();

    CHECK_TRUE(testable.SampleValue());
    CHECK_EQUAL(LogicElement::MaxValue, testable.ReadValue());
    CHECK_FALSE(testable.SampleValue());
}

TEST(LogicControllerDOTestsGroup, UpdateValue_return_true_if_any_changes) {
    TestableControllerDO testable(gpio_output::OUTPUT_0);
    testable.Init();

    CHECK_TRUE(testable.UpdateValue(LogicElement::MaxValue));
    CHECK_FALSE(testable.UpdateValue(LogicElement::MaxValue));
}

TEST(LogicControllerDOTestsGroup, UpdateValue_updated_value) {
    TestableControllerDO testable(gpio_output::OUTPUT_0);
    testable.Init();

    CHECK_TRUE(testable.UpdateValue(LogicElement::MaxValue));
    CHECK_EQUAL(LogicElement::MaxValue, testable.PeekValue());
}

TEST(LogicControllerDOTestsGroup, ReadValue_returns_value) {
    TestableControllerDO testable(gpio_output::OUTPUT_0);
    testable.Init();
    CHECK_TRUE(testable.UpdateValue(LogicElement::MaxValue));

    CHECK_EQUAL(LogicElement::MaxValue, testable.ReadValue());
}

TEST(LogicControllerDOTestsGroup, ReadValue_set_required_reading_and_not_touch_required_writing) {
    TestableControllerDO testable(gpio_output::OUTPUT_0);
    testable.Init();
    *(testable.PublicMorozov_Get_required_reading()) = false;

    testable.ReadValue();
    CHECK_TRUE(*(testable.PublicMorozov_Get_required_reading()));
    CHECK_FALSE(*(testable.PublicMorozov_Get_required_writing()));
}

TEST(LogicControllerDOTestsGroup, WriteValue_changes_out_value) {
    TestableControllerDO testable(gpio_output::OUTPUT_0);

    testable.WriteValue(LogicElement::MaxValue);
    CHECK_EQUAL(LogicElement::MaxValue, *testable.PublicMorozov_Get_out_value());

    testable.WriteValue(42);
    CHECK_EQUAL(42, *testable.PublicMorozov_Get_out_value());
    CHECK_TRUE(*(testable.PublicMorozov_Get_required_writing()));
}

TEST(LogicControllerDOTestsGroup, WriteValue_set_required_writing_and_not_touch_required_reading) {
    TestableControllerDO testable(gpio_output::OUTPUT_0);
    testable.Init();
    *(testable.PublicMorozov_Get_required_reading()) = false;

    testable.WriteValue(42);
    CHECK_TRUE(*(testable.PublicMorozov_Get_required_writing()));
    CHECK_FALSE(*(testable.PublicMorozov_Get_required_reading()));
}

TEST(LogicControllerDOTestsGroup, CommitChanges_reset_required_writing) {
    mock("2").expectOneCall("gpio_set_level").ignoreOtherParameters();
    TestableControllerDO testable(gpio_output::OUTPUT_0);
    testable.Init();
    *(testable.PublicMorozov_Get_required_writing()) = true;

    testable.CommitChanges();
    CHECK_FALSE(*(testable.PublicMorozov_Get_required_writing()));
}

TEST(LogicControllerDOTestsGroup, CommitChanges_updated_out_value) {
    mock("2").expectOneCall("gpio_set_level").withUnsignedIntParameter("level", 1);
    mock("2").expectOneCall("gpio_set_level").withUnsignedIntParameter("level", 0);
    TestableControllerDO testable(gpio_output::OUTPUT_0);
    testable.Init();

    testable.WriteValue(LogicElement::MinValue);
    testable.CommitChanges();

    testable.WriteValue(LogicElement::MaxValue);
    testable.CommitChanges();
}

TEST(LogicControllerDOTestsGroup, Value_changes_in_transaction) {
    mock("2").expectNCalls(1, "gpio_get_level").andReturnValue(1);
    mock("2").expectNCalls(1, "gpio_get_level").andReturnValue(0);
    mock("2").expectOneCall("gpio_set_level").withUnsignedIntParameter("level", 0);
    TestableControllerDO testable(gpio_output::OUTPUT_0);
    testable.Init();

    CHECK_FALSE(testable.SampleValue());
    CHECK_EQUAL(LogicElement::MinValue, testable.ReadValue());

    testable.WriteValue(LogicElement::MaxValue);
    CHECK_EQUAL(LogicElement::MinValue, testable.ReadValue());

    testable.CommitChanges();
    CHECK_EQUAL(LogicElement::MinValue, testable.ReadValue());

    CHECK_TRUE(testable.SampleValue());
    CHECK_EQUAL(LogicElement::MaxValue, testable.ReadValue());
}
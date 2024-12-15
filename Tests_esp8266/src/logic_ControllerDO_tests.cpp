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
        bool *PublicMorozov_Get_required() {
            return &required;
        }
    };
} // namespace

TEST(LogicControllerDOTestsGroup, Init_reset_value_and_set_requried) {
    TestableControllerDO testable(gpio_output::OUTPUT_0);
    *(testable.PublicMorozov_Get_required()) = false;
    testable.UpdateValue(LogicElement::MaxValue);

    testable.Init();
    CHECK_TRUE(*(testable.PublicMorozov_Get_required()));
    CHECK_EQUAL(LogicElement::MinValue, testable.PeekValue());
}

TEST(LogicControllerDOTestsGroup, SampleValue_reset_requried) {
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    TestableControllerDO testable(gpio_output::OUTPUT_0);
    testable.Init();

    CHECK_TRUE(*(testable.PublicMorozov_Get_required()));
    testable.SampleValue();
    CHECK_FALSE(*(testable.PublicMorozov_Get_required()));
}

TEST(LogicControllerDOTestsGroup, SampleValue_return_true_if_any_changes) {
    mock("2").expectNCalls(2, "gpio_get_level").andReturnValue(0);
    TestableControllerDO testable(gpio_output::OUTPUT_0);
    testable.Init();

    CHECK_TRUE(testable.SampleValue());
    CHECK_EQUAL(LogicElement::MaxValue, testable.GetValue());
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

TEST(LogicControllerDOTestsGroup, GetValue_returns_value_and_set_requried) {
    TestableControllerDO testable(gpio_output::OUTPUT_0);
    testable.Init();
    *(testable.PublicMorozov_Get_required()) = false;
    CHECK_TRUE(testable.UpdateValue(LogicElement::MaxValue));

    CHECK_EQUAL(LogicElement::MaxValue, testable.GetValue());
    CHECK_TRUE(*(testable.PublicMorozov_Get_required()));
}

TEST(LogicControllerDOTestsGroup, SetValue_changes_out_value_and_set_requried) {
    mock("2").expectOneCall("gpio_set_level").withUnsignedIntParameter("level", 0);
    TestableControllerDO testable(gpio_output::OUTPUT_0);

    testable.SetValue(LogicElement::MaxValue);
    CHECK_TRUE(*(testable.PublicMorozov_Get_required()));

}

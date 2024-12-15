#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/ControllerDI.h"
#include "main/LogicProgram/LogicElement.h"
#include "sys_gpio.h"

TEST_GROUP(LogicControllerDITestsGroup){ //
                                         TEST_SETUP(){}

                                         TEST_TEARDOWN(){}
};

namespace {
    class TestableControllerDI : public ControllerDI {
      public:
        bool *PublicMorozov_Get_required() {
            return &required;
        }
    };
} // namespace

TEST(LogicControllerDITestsGroup, Init_reset_value_and_set_requried) {
    TestableControllerDI testable;
    *(testable.PublicMorozov_Get_required()) = false;
    testable.UpdateValue(LogicElement::MaxValue);

    testable.Init();
    CHECK_TRUE(*(testable.PublicMorozov_Get_required()));
    CHECK_EQUAL(LogicElement::MinValue, testable.PeekValue());
}

TEST(LogicControllerDITestsGroup, SampleValue_reset_requried) {
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    TestableControllerDI testable;
    testable.Init();

    CHECK_TRUE(*(testable.PublicMorozov_Get_required()));
    testable.SampleValue();
    CHECK_FALSE(*(testable.PublicMorozov_Get_required()));
}

TEST(LogicControllerDITestsGroup, SampleValue_return_true_if_any_changes) {
    mock("0").expectNCalls(2, "gpio_get_level").andReturnValue(0);
    TestableControllerDI testable;
    testable.Init();

    CHECK_TRUE(testable.SampleValue());
    CHECK_EQUAL(LogicElement::MaxValue, testable.GetValue());
    CHECK_FALSE(testable.SampleValue());
}

TEST(LogicControllerDITestsGroup, UpdateValue_return_true_if_any_changes) {
    TestableControllerDI testable;
    testable.Init();

    CHECK_TRUE(testable.UpdateValue(LogicElement::MaxValue));
    CHECK_FALSE(testable.UpdateValue(LogicElement::MaxValue));
}

TEST(LogicControllerDITestsGroup, UpdateValue_updated_value) {
    TestableControllerDI testable;
    testable.Init();

    CHECK_TRUE(testable.UpdateValue(LogicElement::MaxValue));
    CHECK_EQUAL(LogicElement::MaxValue, testable.PeekValue());
}

TEST(LogicControllerDITestsGroup, GetValue_returns_value_and_set_requried) {
    TestableControllerDI testable;
    testable.Init();
    *(testable.PublicMorozov_Get_required()) = false;
    CHECK_TRUE(testable.UpdateValue(LogicElement::MaxValue));

    CHECK_EQUAL(LogicElement::MaxValue, testable.GetValue());
    CHECK_TRUE(*(testable.PublicMorozov_Get_required()));
}

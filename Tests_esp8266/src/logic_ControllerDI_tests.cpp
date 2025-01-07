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
        bool *PublicMorozov_Get_required_reading() {
            return &required_reading;
        }
    };
} // namespace

TEST(LogicControllerDITestsGroup, Init_reset_value_and_set_required) {
    TestableControllerDI testable;
    *(testable.PublicMorozov_Get_required_reading()) = false;
    testable.UpdateValue(LogicElement::MaxValue);

    testable.Init();
    CHECK_TRUE(*(testable.PublicMorozov_Get_required_reading()));
    CHECK_EQUAL(LogicElement::MinValue, testable.PeekValue());
}

TEST(LogicControllerDITestsGroup, FetchValue_reset_required) {
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    TestableControllerDI testable;
    testable.Init();

    CHECK_TRUE(*(testable.PublicMorozov_Get_required_reading()));
    testable.FetchValue();
    CHECK_FALSE(*(testable.PublicMorozov_Get_required_reading()));
}

TEST(LogicControllerDITestsGroup, FetchValue) {
    mock("0").expectNCalls(1, "gpio_get_level").andReturnValue(0);
    TestableControllerDI testable;
    testable.Init();

    testable.FetchValue();
    CHECK_EQUAL(LogicElement::MaxValue, testable.ReadValue());
    CHECK_EQUAL(LogicElement::MaxValue, testable.PeekValue());
}

TEST(LogicControllerDITestsGroup, UpdateValue) {
    TestableControllerDI testable;
    testable.Init();

    testable.UpdateValue(LogicElement::MaxValue);
    CHECK_EQUAL(LogicElement::MaxValue, testable.ReadValue());
    CHECK_EQUAL(LogicElement::MaxValue, testable.PeekValue());
}

TEST(LogicControllerDITestsGroup, ReadValue_returns_value_and_set_required) {
    TestableControllerDI testable;
    testable.Init();
    *(testable.PublicMorozov_Get_required_reading()) = false;
    testable.UpdateValue(LogicElement::MaxValue);

    CHECK_EQUAL(LogicElement::MaxValue, testable.ReadValue());
    CHECK_TRUE(*(testable.PublicMorozov_Get_required_reading()));
}

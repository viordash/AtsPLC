#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/InputNC.h"
#include "main/LogicProgram/Outputs/IncOutput.h"

TEST_GROUP(LogicIncOutputTestsGroup){ //
                                      TEST_SETUP(){}

                                      TEST_TEARDOWN(){}
};

namespace {
    class TestableIncOutput : public IncOutput {
      public:
        TestableIncOutput(const MapIO io_adr) : IncOutput(io_adr) {
        }
        virtual ~TestableIncOutput() {
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
    };
} // namespace

TEST(LogicIncOutputTestsGroup, DoAction_skip_when_incoming_passive) {
    TestableIncOutput testable(MapIO::V1);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicIncOutputTestsGroup,
     DoAction_change_state_to_active__and_second_call_does_not_decrement) {
    TestableIncOutput testable(MapIO::V1);

    Controller::SetV1RelativeValue(42);

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_EQUAL(43, Controller::GetV1RelativeValue());

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(43, Controller::GetV1RelativeValue());
}

TEST(LogicIncOutputTestsGroup, DoAction_change_state_to_passive) {

    Controller::SetV1RelativeValue(42);

    TestableIncOutput testable(MapIO::V1);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_EQUAL(42, Controller::GetV1RelativeValue());
}

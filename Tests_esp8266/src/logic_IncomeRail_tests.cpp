#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/IncomeRail.h"
#include "main/LogicProgram/LogicProgram.h"

TEST_GROUP(LogicIncomeRailTestsGroup){ //
                                       TEST_SETUP(){ mock().disable();
}

TEST_TEARDOWN() {
    mock().enable();
}
}
;

namespace {
    class PublicMorozov_Next {
      public:
        LogicElement *PublicMorozov_GetNext = 0;
    };

    class TestableIncomeRail : public IncomeRail, public PublicMorozov_Next {
      public:
        TestableIncomeRail(const Controller &controller, uint8_t network_number)
            : IncomeRail(controller, network_number) {
        }
        LogicElement *PublicMorozov_GetNext() {
            return next;
        }
    };

    class TestableInputNO : public InputNO, public PublicMorozov_Next {
      public:
        TestableInputNO(const MapIO io_adr, InputBase *incoming_item)
            : InputNO(io_adr, incoming_item) {
        }
        LogicElement *PublicMorozov_GetNext() {
            return next;
        }
    };

    class TestableComparatorGE : public ComparatorGE, public PublicMorozov_Next {
      public:
        TestableComparatorGE(uint16_t reference, const MapIO io_adr, InputBase *incoming_item)
            : ComparatorGE(reference, io_adr, incoming_item) {
        }
        LogicElement *PublicMorozov_GetNext() {
            return next;
        }
    };

    class TestableTimerSecs : public TimerSecs, public PublicMorozov_Next {
      public:
        TestableTimerSecs(uint32_t delay_time_s, InputBase *incoming_item)
            : TimerSecs(delay_time_s, incoming_item) {
        }
        LogicElement *PublicMorozov_GetNext() {
            return next;
        }
    };

    class TestableDirectOutput : public DirectOutput, public PublicMorozov_Next {
      public:
        TestableDirectOutput(const MapIO io_adr, InputBase *incoming_item)
            : DirectOutput(io_adr, incoming_item) {
        }
        LogicElement *PublicMorozov_GetNext() {
            return next;
        }
    };

} // namespace

TEST(LogicIncomeRailTestsGroup, Chain_of_logic_elements) {

    Controller controller;
    TestableIncomeRail testable(controller, 0);

    TestableInputNO input1(MapIO::DI, &testable);
    TestableComparatorGE comparator1(5, MapIO::AI, &input1);
    TestableTimerSecs timerSecs1(2, &comparator1);
    TestableDirectOutput directOutput0(MapIO::O1, &timerSecs1);
    OutcomeRail outcomeRail0(0);

    LogicElement *next = testable.PublicMorozov_GetNext();
    CHECK_EQUAL(&input1, next);

    next = input1.PublicMorozov_GetNext();
    CHECK_EQUAL(&comparator1, next);

    next = comparator1.PublicMorozov_GetNext();
    CHECK_EQUAL(&timerSecs1, next);

    next = timerSecs1.PublicMorozov_GetNext();
    CHECK_EQUAL(&directOutput0, next);

    next = directOutput0.PublicMorozov_GetNext();
    CHECK(next == NULL);
}

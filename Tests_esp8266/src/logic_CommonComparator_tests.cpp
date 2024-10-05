#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/CommonComparator.h"
#include "main/LogicProgram/Inputs/ComparatorEq.h"
#include "main/LogicProgram/Inputs/ComparatorGE.h"
#include "main/LogicProgram/Inputs/ComparatorGr.h"
#include "main/LogicProgram/Inputs/ComparatorLE.h"
#include "main/LogicProgram/Inputs/ComparatorLs.h"
#include "main/LogicProgram/Inputs/InputNC.h"
#include "main/LogicProgram/Inputs/InputNO.h"
#include "main/LogicProgram/Inputs/TimerMSecs.h"
#include "main/LogicProgram/Inputs/TimerSecs.h"
#include "main/LogicProgram/Outputs/DecOutput.h"
#include "main/LogicProgram/Outputs/DirectOutput.h"
#include "main/LogicProgram/Outputs/IncOutput.h"
#include "main/LogicProgram/Outputs/ResetOutput.h"
#include "main/LogicProgram/Outputs/SetOutput.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};
TEST_GROUP(LogicCommonComparatorTestsGroup){
    //
    TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));
mock().disable();
}

TEST_TEARDOWN() {
    mock().enable();
}
}
;

namespace {
    static const Bitmap bitmap = { //
        { 16,                      // width
          16 },                    // height
        { 0xFF, 0x00, 0x00, 0x0A, 0x0A, 0x0A, 0x0A, 0x00, 0x0A, 0x0A, 0x0A,
          0x0A, 0x00, 0x00, 0xFF, 0x80, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x01 }
    };

    class TestableCommonComparator : public CommonComparator {
      public:
        TvElementType elementType;

        TestableCommonComparator(TvElementType elementType = TvElementType::et_ComparatorLs)
            : CommonComparator() {
            this->elementType = elementType;
        }
        virtual ~TestableCommonComparator() {
        }

        const Bitmap *GetCurrentBitmap(LogicItemState state) {
            (void)state;
            return &bitmap;
        }

        bool CompareFunction() override {
            return true;
        }
        MapIO PublicMorozov_Get_io_adr() {
            return io_adr;
        }
        uint8_t PublicMorozov_GetReference() {
            return ref_percent04;
        }
        const char *PublicMorozov_GetStrReference() {
            return str_reference;
        }
        TvElementType GetElementType() override final {
            return elementType;
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
    };
} // namespace

TEST(LogicCommonComparatorTestsGroup, Reference_in_limit_0_to_250) {
    TestableCommonComparator testable_0;
    CHECK_EQUAL(0, testable_0.PublicMorozov_GetReference());

    TestableCommonComparator testable_100;
    testable_100.SetReference(100);
    CHECK_EQUAL(100, testable_100.PublicMorozov_GetReference());

    TestableCommonComparator testable_250;
    testable_250.SetReference(250);
    CHECK_EQUAL(250, testable_250.PublicMorozov_GetReference());

    TestableCommonComparator testable_251;
    testable_251.SetReference(251);
    CHECK_EQUAL(250, testable_251.PublicMorozov_GetReference());
}

TEST(LogicCommonComparatorTestsGroup, Render) {
    TestableCommonComparator testable;
    testable.SetIoAdr(MapIO::AI);

    Point start_point = { 0, INCOME_RAIL_TOP };
    CHECK_TRUE(testable.Render(frame_buffer, LogicItemState::lisActive, &start_point));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
    CHECK_EQUAL(32, start_point.x);
}

TEST(LogicCommonComparatorTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableCommonComparator testable;
    testable.SetReference(42);
    testable.SetIoAdr(MapIO::V2);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(3, writed);

    CHECK_EQUAL(TvElementType::et_ComparatorLs, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(42, *((uint8_t *)&buffer[1]));
    CHECK_EQUAL(MapIO::V2, *((MapIO *)&buffer[2]));
}

TEST(LogicCommonComparatorTestsGroup, Serialize_just_for_obtain_size) {
    TestableCommonComparator testable;
    testable.SetReference(50 / 0.4);
    testable.SetIoAdr(MapIO::AI);

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(3, writed);

    writed = testable.Serialize(NULL, 0);
    CHECK_EQUAL(3, writed);
}

TEST(LogicCommonComparatorTestsGroup, Serialize_to_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    TestableCommonComparator testable;
    testable.SetReference(50 / 0.4);
    testable.SetIoAdr(MapIO::AI);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicCommonComparatorTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_ComparatorLs;
    *((uint8_t *)&buffer[1]) = 42;
    *((MapIO *)&buffer[2]) = MapIO::V3;

    TestableCommonComparator testable;
    testable.SetReference(19);

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(2, readed);

    CHECK_EQUAL(42, testable.PublicMorozov_GetReference());
    CHECK_EQUAL(MapIO::V3, testable.PublicMorozov_Get_io_adr());
}

TEST(LogicCommonComparatorTestsGroup, Deserialize_with_small_buffer_return_zero) {
    uint8_t buffer[0] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_ComparatorLs;

    TestableCommonComparator testable;
    testable.SetReference(50 / 0.4);

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

TEST(LogicCommonComparatorTestsGroup, Deserialize_with_wrong_io_adr_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_ComparatorLs;
    *((uint8_t *)&buffer[1]) = 42;

    TestableCommonComparator testable;
    testable.SetReference(50 / 0.4);

    *((MapIO *)&buffer[2]) = (MapIO)(MapIO::DI - 1);
    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    *((MapIO *)&buffer[2]) = (MapIO)(MapIO::V4 + 1);
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    *((MapIO *)&buffer[2]) = MapIO::DI;
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(2, readed);
}

TEST(LogicCommonComparatorTestsGroup, Deserialize_with_large_reference_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_ComparatorLs;
    *((MapIO *)&buffer[2]) = MapIO::V3;

    TestableCommonComparator testable;
    testable.SetIoAdr(MapIO::V2);

    *((uint8_t *)&buffer[1]) = LogicElement::MaxValue + 1;
    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    *((uint8_t *)&buffer[1]) = LogicElement::MaxValue;
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(2, readed);
}

TEST(LogicCommonComparatorTestsGroup,
     DoAction_change_state_to_passive__due_incoming_switch_to_passive) {

    TestableCommonComparator testable;
    testable.SetIoAdr(MapIO::V2);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());

    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());

    CHECK_FALSE_TEXT(testable.DoAction(true, LogicItemState::lisPassive),
                     "no changes are expected to be detected");
    CHECK_FALSE_TEXT(testable.DoAction(false, LogicItemState::lisPassive),
                     "no changes are expected to be detected");
}

TEST(LogicCommonComparatorTestsGroup, ctor_set_reference_to_zero) {
    TestableCommonComparator testable;
    CHECK_EQUAL(0, testable.PublicMorozov_GetReference());
    STRCMP_EQUAL("0", testable.PublicMorozov_GetStrReference());
}

TEST(LogicCommonComparatorTestsGroup, TryToCast) {
    InputNC inputNC;
    CHECK_TRUE(CommonComparator::TryToCast(&inputNC) == NULL);

    InputNO inputNO;
    CHECK_TRUE(CommonComparator::TryToCast(&inputNO) == NULL);

    ComparatorEq comparatorEq;
    CHECK_TRUE(CommonComparator::TryToCast(&comparatorEq) == &comparatorEq);

    ComparatorGE comparatorGE;
    CHECK_TRUE(CommonComparator::TryToCast(&comparatorGE) == &comparatorGE);

    ComparatorGr comparatorGr;
    CHECK_TRUE(CommonComparator::TryToCast(&comparatorGr) == &comparatorGr);

    ComparatorLE comparatorLE;
    CHECK_TRUE(CommonComparator::TryToCast(&comparatorLE) == &comparatorLE);

    ComparatorLs comparatorLs;
    CHECK_TRUE(CommonComparator::TryToCast(&comparatorLs) == &comparatorLs);

    TimerMSecs timerMSecs;
    CHECK_TRUE(CommonComparator::TryToCast(&timerMSecs) == NULL);

    TimerSecs timerSecs;
    CHECK_TRUE(CommonComparator::TryToCast(&timerSecs) == NULL);

    DirectOutput directOutput;
    CHECK_TRUE(CommonComparator::TryToCast(&directOutput) == NULL);

    SetOutput setOutput;
    CHECK_TRUE(CommonComparator::TryToCast(&setOutput) == NULL);

    ResetOutput resetOutput;
    CHECK_TRUE(CommonComparator::TryToCast(&resetOutput) == NULL);

    IncOutput incOutput;
    CHECK_TRUE(CommonComparator::TryToCast(&incOutput) == NULL);

    DecOutput decOutput;
    CHECK_TRUE(CommonComparator::TryToCast(&decOutput) == NULL);
}

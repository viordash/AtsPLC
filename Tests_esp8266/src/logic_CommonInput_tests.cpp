#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/CommonInput.h"
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

static FrameBuffer frame_buffer = {};

TEST_GROUP(LogicCommonInputTestsGroup){
    //
    TEST_SETUP(){ memset(&frame_buffer.buffer, 0, sizeof(frame_buffer.buffer));
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

    class TestableCommonInput : public CommonInput {
      public:
        TestableCommonInput() : CommonInput() {
        }
        virtual ~TestableCommonInput() {
        }
        const Bitmap *GetCurrentBitmap(LogicItemState state) {
            (void)state;
            return &bitmap;
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
        bool DoAction(bool prev_changed, LogicItemState prev_elem_state) {
            (void)prev_changed;
            (void)prev_elem_state;
            return true;
        }

        size_t Serialize(uint8_t *buffer, size_t buffer_size) override {
            (void)buffer;
            (void)buffer_size;
            return 0;
        }
        size_t Deserialize(uint8_t *buffer, size_t buffer_size) override {
            (void)buffer;
            (void)buffer_size;
            return 0;
        }
        TvElementType GetElementType() override {
            return TvElementType::et_Undef;
        }
        const AllowedIO GetAllowedInputs() {
            static MapIO allowedIO[] = { MapIO::DI, MapIO::AI, MapIO::V1,
                                         MapIO::V2, MapIO::V3, MapIO::V4 };
            return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
        }
    };
} // namespace

TEST(LogicCommonInputTestsGroup, Render_when_active) {
    TestableCommonInput testable;
    testable.SetIoAdr(MapIO::DI);

    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    Point start_point = { 0, INCOME_RAIL_TOP };
    testable.Render(&frame_buffer, LogicItemState::lisActive, &start_point);

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer.buffer); i++) {
        if (frame_buffer.buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
    CHECK_EQUAL(32, start_point.x);
}

TEST(LogicCommonInputTestsGroup, Render_when_passive) {
    TestableCommonInput testable;
    testable.SetIoAdr(MapIO::DI);

    Point start_point = { 0, INCOME_RAIL_TOP };
    testable.Render(&frame_buffer, LogicItemState::lisActive, &start_point);

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer.buffer); i++) {
        if (frame_buffer.buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
    CHECK_EQUAL(32, start_point.x);
}

TEST(LogicCommonInputTestsGroup, TryToCast) {
    InputNC inputNC;
    CHECK_TRUE(CommonInput::TryToCast(&inputNC) == &inputNC);

    InputNO inputNO;
    CHECK_TRUE(CommonInput::TryToCast(&inputNO) == &inputNO);

    ComparatorEq comparatorEq;
    CHECK_TRUE(CommonInput::TryToCast(&comparatorEq) == &comparatorEq);

    ComparatorGE comparatorGE;
    CHECK_TRUE(CommonInput::TryToCast(&comparatorGE) == &comparatorGE);

    ComparatorGr comparatorGr;
    CHECK_TRUE(CommonInput::TryToCast(&comparatorGr) == &comparatorGr);

    ComparatorLE comparatorLE;
    CHECK_TRUE(CommonInput::TryToCast(&comparatorLE) == &comparatorLE);

    ComparatorLs comparatorLs;
    CHECK_TRUE(CommonInput::TryToCast(&comparatorLs) == &comparatorLs);

    TimerMSecs timerMSecs;
    CHECK_TRUE(CommonInput::TryToCast(&timerMSecs) == NULL);

    TimerSecs timerSecs;
    CHECK_TRUE(CommonInput::TryToCast(&timerSecs) == NULL);

    DirectOutput directOutput;
    CHECK_TRUE(CommonInput::TryToCast(&directOutput) == NULL);

    SetOutput setOutput;
    CHECK_TRUE(CommonInput::TryToCast(&setOutput) == NULL);

    ResetOutput resetOutput;
    CHECK_TRUE(CommonInput::TryToCast(&resetOutput) == NULL);

    IncOutput incOutput;
    CHECK_TRUE(CommonInput::TryToCast(&incOutput) == NULL);

    DecOutput decOutput;
    CHECK_TRUE(CommonInput::TryToCast(&decOutput) == NULL);
}

TEST(LogicCommonInputTestsGroup, SelectNext_changing_IoAdr) {
    TestableCommonInput testable;
    testable.SetIoAdr(MapIO::DI);
    testable.BeginEditing();
    testable.SelectNext();
    CHECK_EQUAL(MapIO::AI, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V1, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V2, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V3, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V4, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::DI, testable.GetIoAdr());
}

TEST(LogicCommonInputTestsGroup, SelectPrior_changing_IoAdr) {
    TestableCommonInput testable;
    testable.SetIoAdr(MapIO::DI);
    testable.BeginEditing();
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V4, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V3, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V2, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V1, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::AI, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::DI, testable.GetIoAdr());
}

TEST(LogicCommonInputTestsGroup, second_Change_calls_end_editing) {
    TestableCommonInput testable;
    testable.SetIoAdr(MapIO::O1);
    testable.BeginEditing();
    CHECK_TRUE(testable.Editing());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    testable.Change();
    CHECK_FALSE(testable.Editing());
}
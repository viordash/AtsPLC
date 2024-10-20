#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/InputNC.h"
#include "main/LogicProgram/Outputs/CommonOutput.h"
#include "main/LogicProgram/Outputs/DecOutput.h"
#include "main/LogicProgram/Outputs/DirectOutput.h"
#include "main/LogicProgram/Outputs/IncOutput.h"
#include "main/LogicProgram/Outputs/ResetOutput.h"
#include "main/LogicProgram/Outputs/SetOutput.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicCommonOutputTestsGroup){
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

    class TestableCommonOutput : public CommonOutput {
      public:
        TestableCommonOutput() : CommonOutput() {
        }
        virtual ~TestableCommonOutput() {
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
        TvElementType GetElementType() override {
            return TvElementType::et_Undef;
        }
        const AllowedIO GetAllowedOutputs() {
            static MapIO allowedIO[] = { MapIO::O1, MapIO::O2, MapIO::V1,
                                         MapIO::V2, MapIO::V3, MapIO::V4 };
            return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
        }
    };
} // namespace

TEST(LogicCommonOutputTestsGroup, TryToCast) {
    DirectOutput directOutput;
    CHECK_TRUE(CommonOutput::TryToCast(&directOutput) == &directOutput);

    SetOutput setOutput;
    CHECK_TRUE(CommonOutput::TryToCast(&setOutput) == &setOutput);

    ResetOutput resetOutput;
    CHECK_TRUE(CommonOutput::TryToCast(&resetOutput) == &resetOutput);

    IncOutput incOutput;
    CHECK_TRUE(CommonOutput::TryToCast(&incOutput) == &incOutput);

    DecOutput decOutput;
    CHECK_TRUE(CommonOutput::TryToCast(&decOutput) == &decOutput);

    InputNC inputNC;
    CHECK_TRUE(CommonOutput::TryToCast(&inputNC) == NULL);
}

TEST(LogicCommonOutputTestsGroup, SelectNext_changing_IoAdr) {
    TestableCommonOutput testable;
    testable.SetIoAdr(MapIO::O1);
    testable.BeginEditing();
    testable.SelectNext();
    CHECK_EQUAL(MapIO::O2, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V1, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V2, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V3, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V4, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::O1, testable.GetIoAdr());
}

TEST(LogicCommonOutputTestsGroup, SelectPrior_changing_IoAdr) {
    TestableCommonOutput testable;
    testable.SetIoAdr(MapIO::O1);
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
    CHECK_EQUAL(MapIO::O2, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::O1, testable.GetIoAdr());
}

TEST(LogicCommonOutputTestsGroup, second_Change_calls_end_editing) {
    TestableCommonOutput testable;
    testable.SetIoAdr(MapIO::O1);
    testable.BeginEditing();
    CHECK_TRUE(testable.Editing());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    testable.Change();
    CHECK_FALSE(testable.Editing());
}

TEST(LogicCommonOutputTestsGroup, Render_when_active) {
    TestableCommonOutput testable;
    testable.SetIoAdr(MapIO::O1);

    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    Point start_point = { OUTCOME_RAIL_RIGHT, INCOME_RAIL_TOP };
    CHECK_TRUE(testable.Render(frame_buffer, LogicItemState::lisActive, &start_point));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
}

TEST(LogicCommonOutputTestsGroup, Render_update_start_point_with_most_left_point) {
    TestableCommonOutput testable;
    testable.SetIoAdr(MapIO::O1);

    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    Point start_point = { OUTCOME_RAIL_RIGHT, INCOME_RAIL_TOP };
    CHECK_TRUE(testable.Render(frame_buffer, LogicItemState::lisActive, &start_point));

    // CHECK_EQUAL(32, start_point.x);
}
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

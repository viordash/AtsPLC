#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/CommonInput.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicCommonInputTestsGroup){ //
                                        TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));
}

TEST_TEARDOWN() {
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
    };
} // namespace

TEST(LogicCommonInputTestsGroup, Render_when_active) {
    TestableCommonInput testable;
    testable.SetIoAdr(MapIO::DI);

    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

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

TEST(LogicCommonInputTestsGroup, Render_when_passive) {
    TestableCommonInput testable;
    testable.SetIoAdr(MapIO::DI);

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

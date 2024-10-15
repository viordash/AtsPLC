#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/ComparatorEq.h"
#include "main/LogicProgram/Inputs/ComparatorGE.h"
#include "main/LogicProgram/Inputs/ComparatorGr.h"
#include "main/LogicProgram/Inputs/ComparatorLE.h"
#include "main/LogicProgram/Inputs/ComparatorLs.h"
#include "main/LogicProgram/Inputs/InputNO.h"
#include "main/LogicProgram/Wire.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicWireTestsGroup){ //
                                 TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));

mock().expectOneCall("vTaskDelay").ignoreOtherParameters();
Controller::Stop();
}

TEST_TEARDOWN() {
}
}
;

namespace {
    class TestableWire : public Wire {
      public:
        TestableWire() : Wire() {
        }
        virtual ~TestableWire() {
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
    };
} // namespace

TEST(LogicWireTestsGroup, DoAction_copied_prev_elem_changed) {
    TestableWire testable;

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive));
}

TEST(LogicWireTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableWire testable;

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(1, writed);

    CHECK_EQUAL(TvElementType::et_Wire, *((TvElementType *)&buffer[0]));
}

TEST(LogicWireTestsGroup, Serialize_just_for_obtain_size) {
    TestableWire testable;

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(1, writed);

    writed = testable.Serialize(NULL, 0);
    CHECK_EQUAL(1, writed);
}

TEST(LogicWireTestsGroup, Serialize_to_small_buffer_return_zero) {
    uint8_t buffer[0] = {};
    TestableWire testable;

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicWireTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_Wire;

    TestableWire testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);
}

TEST(LogicWireTestsGroup, Deserialize_with_small_buffer_return_zero) {
    uint8_t buffer[0] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_Wire;

    TestableWire testable;

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

TEST(LogicWireTestsGroup, GetElementType) {
    TestableWire testable;
    CHECK_EQUAL(TvElementType::et_Wire, testable.GetElementType());
}
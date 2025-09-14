#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Controller.h"
#include "main/LogicProgram/Flow/ContinuationIn.h"
#include "main/LogicProgram/Flow/ContinuationOut.h"

static FrameBuffer frame_buffer = {};

TEST_GROUP(LogicContinuationOutTestsGroup){
    //
    TEST_SETUP(){ memset(&frame_buffer.buffer, 0, sizeof(frame_buffer.buffer));
}

TEST_TEARDOWN() {
}
}
;

TEST(LogicContinuationOutTestsGroup, Render) {
    ContinuationOut testable;

    Point start_point = { INCOME_RAIL_WIDTH, INCOME_RAIL_TOP + INCOME_RAIL_NETWORK_TOP };
    CHECK_TRUE(testable.Render(&frame_buffer, LogicItemState::lisActive, &start_point));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer.buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
    CHECK_EQUAL(24, start_point.x);
}

TEST(LogicContinuationOutTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    ContinuationOut testable;

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(2, writed);

    CHECK_EQUAL(TvElementType::et_ContinuationOut, *((TvElementType *)&buffer[0]));
}

TEST(LogicContinuationOutTestsGroup, Serialize_just_for_obtain_size) {
    ContinuationOut testable;

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(2, writed);

    writed = testable.Serialize(NULL, 0);
    CHECK_EQUAL(2, writed);
}

TEST(LogicContinuationOutTestsGroup, Serialize_to_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    ContinuationOut testable;

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicContinuationOutTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_ContinuationOut;

    ContinuationOut testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(1, readed);
}

TEST(LogicContinuationOutTestsGroup, Deserialize_with_small_buffer_return_zero) {
    uint8_t buffer[0] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_ContinuationOut;

    ContinuationOut testable;

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

TEST(LogicContinuationOutTestsGroup, GetElementType) {
    ContinuationOut testable;
    CHECK_EQUAL(TvElementType::et_ContinuationOut, testable.GetElementType());
}

TEST(LogicContinuationOutTestsGroup, TryToCast) {
    ContinuationOut testable;
    CHECK(ContinuationOut::TryToCast(&testable) == &testable);

    ContinuationIn other;
    CHECK(ContinuationOut::TryToCast(&other) == NULL);
}

TEST(LogicContinuationOutTestsGroup, DoAction_copy_states_from_Controller_network_continuation) {
    ContinuationOut testable;
    Controller::SetNetworkContinuation(LogicItemState::lisActive);

    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, testable.GetState());

    Controller::SetNetworkContinuation(LogicItemState::lisPassive);
    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisPassive, testable.GetState());
}

TEST(
    LogicContinuationOutTestsGroup,
    DoAction_remains_in_passive_regardless_of_Controller_network_continuation_if_prev_element_is_passive) {
    ContinuationOut testable;
    Controller::SetNetworkContinuation(LogicItemState::lisActive);

    CHECK_FALSE(testable.DoAction(true, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, testable.GetState());
}

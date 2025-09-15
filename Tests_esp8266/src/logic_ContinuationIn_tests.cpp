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

TEST_GROUP(LogicContinuationInTestsGroup){
    //
    TEST_SETUP(){ memset(&frame_buffer.buffer, 0, sizeof(frame_buffer.buffer));
}

TEST_TEARDOWN() {
}
}
;

TEST(LogicContinuationInTestsGroup, Render) {
    ContinuationIn testable;

    Point start_point = { OUTCOME_RAIL_RIGHT, INCOME_RAIL_TOP };
    testable.Render(&frame_buffer, LogicItemState::lisActive, &start_point);

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer.buffer); i++){
        if (frame_buffer.buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
    CHECK_EQUAL(108, start_point.x);
}

TEST(LogicContinuationInTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    ContinuationIn testable;

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(2, writed);

    CHECK_EQUAL(TvElementType::et_ContinuationIn, *((TvElementType *)&buffer[0]));
}

TEST(LogicContinuationInTestsGroup, Serialize_just_for_obtain_size) {
    ContinuationIn testable;

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(2, writed);

    writed = testable.Serialize(NULL, 0);
    CHECK_EQUAL(2, writed);
}

TEST(LogicContinuationInTestsGroup, Serialize_to_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    ContinuationIn testable;

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicContinuationInTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_ContinuationIn;

    ContinuationIn testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(1, readed);
}

TEST(LogicContinuationInTestsGroup, Deserialize_with_small_buffer_return_zero) {
    uint8_t buffer[0] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_ContinuationIn;

    ContinuationIn testable;

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

TEST(LogicContinuationInTestsGroup, GetElementType) {
    ContinuationIn testable;
    CHECK_EQUAL(TvElementType::et_ContinuationIn, testable.GetElementType());
}

TEST(LogicContinuationInTestsGroup, TryToCast) {
    ContinuationIn testable;
    CHECK(ContinuationIn::TryToCast(&testable) == &testable);

    ContinuationOut other;
    CHECK(ContinuationIn::TryToCast(&other) == NULL);
}

TEST(LogicContinuationInTestsGroup, DoAction_changes_Controller_network_continuation_in_any_cases) {
    ContinuationIn testable;
    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisActive).any_changes);
    CHECK_EQUAL(LogicItemState::lisActive, Controller::GetNetworkContinuation());

    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive).any_changes);
    CHECK_EQUAL(LogicItemState::lisPassive, Controller::GetNetworkContinuation());

    testable.DoAction(false, LogicItemState::lisActive);
    CHECK_EQUAL(LogicItemState::lisActive, Controller::GetNetworkContinuation());

    testable.DoAction(false, LogicItemState::lisPassive);
    CHECK_EQUAL(LogicItemState::lisPassive, Controller::GetNetworkContinuation());
}

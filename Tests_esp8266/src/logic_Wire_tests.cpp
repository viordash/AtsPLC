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

TEST(LogicWireTestsGroup, DoAction_copied_prev_elem_changed) {
    Wire testable;

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive));
}

TEST(LogicWireTestsGroup, GetElementType) {
    Wire testable;
    CHECK_EQUAL(TvElementType::et_Wire, testable.GetElementType());
}
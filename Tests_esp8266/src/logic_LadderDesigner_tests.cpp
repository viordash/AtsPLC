#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "MonitorLogicElement.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/LogicProgram.h"
#include "main/redundant_storage.h"

#include "tests_utils.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicLadderDesignerTestsGroup){ //
                                           TEST_SETUP(){ mock().disable();
memset(frame_buffer, 0, sizeof(frame_buffer));
create_storage_0();
create_storage_1();
}

TEST_TEARDOWN() {
    mock().enable();
    remove_storage_0();
    remove_storage_1();
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

    class TestableLadder : public Ladder {
      public:
        int *PublicMorozov_Get_view_top_index() {
            return &view_top_index;
        }
        int PublicMorozov_GetSelectedNetwork() {
            return GetSelectedNetwork();
        }
        EditableElement::ElementState PublicMorozov_GetDesignState(int selected_network) {
            return GetDesignState(selected_network);
        }
    };

} // namespace

TEST(LogicLadderDesignerTestsGroup, CanScrollAuto_after_appending_second_network) {
    TestableLadder testable;

    CHECK_FALSE(testable.CanScrollAuto());
    testable.Append(new Network());

    CHECK_FALSE(testable.CanScrollAuto());
    testable.Append(new Network());

    CHECK_TRUE(testable.CanScrollAuto());
}

TEST(LogicLadderDesignerTestsGroup, AutoScroll_when_append_new_networks) {
    TestableLadder testable;

    testable.AutoScroll();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.Append(new Network());
    testable.AutoScroll();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.Append(new Network());
    testable.AutoScroll();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.Append(new Network());
    testable.AutoScroll();
    CHECK_EQUAL(1, *testable.PublicMorozov_Get_view_top_index());

    testable.Append(new Network());
    testable.AutoScroll();
    CHECK_EQUAL(2, *testable.PublicMorozov_Get_view_top_index());
}

TEST(LogicLadderDesignerTestsGroup,
     HandleButtonDown_HandleButtonUp_can_scroll_from_first_to_last_network) {
    TestableLadder testable;
    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));
    testable.AutoScroll();

    CHECK_EQUAL(5, *testable.PublicMorozov_Get_view_top_index());
    testable.HandleButtonDown();
    CHECK_EQUAL(5, *testable.PublicMorozov_Get_view_top_index());

    testable.HandleButtonUp();
    CHECK_EQUAL(4, *testable.PublicMorozov_Get_view_top_index());

    testable.HandleButtonUp();
    CHECK_EQUAL(3, *testable.PublicMorozov_Get_view_top_index());

    testable.HandleButtonUp();
    CHECK_EQUAL(2, *testable.PublicMorozov_Get_view_top_index());

    testable.HandleButtonUp();
    CHECK_EQUAL(1, *testable.PublicMorozov_Get_view_top_index());

    testable.HandleButtonUp();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.HandleButtonUp();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.HandleButtonDown();
    CHECK_EQUAL(1, *testable.PublicMorozov_Get_view_top_index());
}

TEST(LogicLadderDesignerTestsGroup,
     HandleButtonDown_HandleButtonUp__when_networks_less_than_viewport) {
    TestableLadder testable;
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.AutoScroll();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());
    testable.HandleButtonDown();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());
    testable.HandleButtonUp();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.Append(new Network(LogicItemState::lisActive));

    testable.AutoScroll();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());
    testable.HandleButtonDown();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());
    testable.HandleButtonUp();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.Append(new Network(LogicItemState::lisActive));

    testable.AutoScroll();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());
    testable.HandleButtonDown();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());
    testable.HandleButtonUp();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.Append(new Network(LogicItemState::lisActive));

    testable.AutoScroll();
    CHECK_EQUAL(1, *testable.PublicMorozov_Get_view_top_index());
    testable.HandleButtonDown();
    CHECK_EQUAL(1, *testable.PublicMorozov_Get_view_top_index());
    testable.HandleButtonUp();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());
}

TEST(LogicLadderDesignerTestsGroup, GetSelectedNetwork) {
    TestableLadder testable;
    testable.Append(new Network());
    testable.Append(new Network());
    auto can_be_selected_network = new Network();
    testable.Append(can_be_selected_network);
    testable.Append(new Network());

    CHECK_EQUAL_TEXT(-1,
                     testable.PublicMorozov_GetSelectedNetwork(),
                     "when no selection must be return -1");

    can_be_selected_network->Select();

    CHECK_EQUAL(2, testable.PublicMorozov_GetSelectedNetwork());
}

TEST(LogicLadderDesignerTestsGroup, GetDesignState) {
    TestableLadder testable;
    testable.Append(new Network());
    auto can_be_edited_network = new Network();
    testable.Append(can_be_edited_network);
    auto can_be_selected_network = new Network();
    testable.Append(can_be_selected_network);

    CHECK_EQUAL(EditableElement::ElementState::des_Regular, testable.PublicMorozov_GetDesignState(-1));
    CHECK_EQUAL(EditableElement::ElementState::des_Regular, testable.PublicMorozov_GetDesignState(0));
    CHECK_EQUAL(EditableElement::ElementState::des_Regular, testable.PublicMorozov_GetDesignState(1));
    CHECK_EQUAL(EditableElement::ElementState::des_Regular, testable.PublicMorozov_GetDesignState(2));

    can_be_selected_network->Select();
    CHECK_EQUAL(EditableElement::ElementState::des_Selected, testable.PublicMorozov_GetDesignState(2));

    can_be_edited_network->Select();
    can_be_edited_network->BeginEditing();
    CHECK_EQUAL(EditableElement::ElementState::des_Editing, testable.PublicMorozov_GetDesignState(1));
}
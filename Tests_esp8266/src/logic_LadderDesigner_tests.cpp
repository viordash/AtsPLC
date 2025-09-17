#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "MonitorLogicElement.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "hotreload_service.h"
#include "main/LogicProgram/Ladder.h"
#include "main/LogicProgram/LogicProgram.h"
#include "main/redundant_storage.h"

#include "tests_utils.h"

static uint8_t frame_buffer[DISPLAY_HEIGHT_IN_BYTES * DISPLAY_WIDTH] = {};

TEST_GROUP(LogicLadderDesignerTestsGroup){ //
                                           TEST_SETUP(){ mock().disable();
memset(frame_buffer, 0, sizeof(frame_buffer));
create_storage_0();
create_storage_1();
load_hotreload();
Controller::Start(NULL, NULL, NULL, NULL);
Controller::UpdateUIViewTop(0);
Controller::UpdateUISelected(0);
}

TEST_TEARDOWN() {
    Controller::Stop();
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
        TestableLadder() : Ladder() {
        }

        int32_t *PublicMorozov_Get_view_top_index() {
            return &view_top_index;
        }
        int PublicMorozov_GetSelectedNetwork() {
            return GetSelectedNetwork();
        }
        EditableElement::ElementState PublicMorozov_GetDesignState(int selected_network) {
            return GetDesignState(selected_network);
        }
        bool PublicMorozov_RemoveNetworkIfEmpty(int network_id) {
            return RemoveNetworkIfEmpty(network_id);
        }
    };

    class TestableNetwork : public Network {
      public:
        TestableNetwork(LogicItemState state) : Network(state) {
        }
        LogicItemState PublicMorozov_state() {
            return state;
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

    CHECK_EQUAL(EditableElement::ElementState::des_Regular,
                testable.PublicMorozov_GetDesignState(-1));
    CHECK_EQUAL(EditableElement::ElementState::des_Regular,
                testable.PublicMorozov_GetDesignState(0));
    CHECK_EQUAL(EditableElement::ElementState::des_Regular,
                testable.PublicMorozov_GetDesignState(1));
    CHECK_EQUAL(EditableElement::ElementState::des_Regular,
                testable.PublicMorozov_GetDesignState(2));

    can_be_selected_network->Select();
    CHECK_EQUAL(EditableElement::ElementState::des_Selected,
                testable.PublicMorozov_GetDesignState(2));

    can_be_edited_network->Select();
    can_be_edited_network->BeginEditing();
    CHECK_EQUAL(EditableElement::ElementState::des_Editing,
                testable.PublicMorozov_GetDesignState(1));
}

TEST(LogicLadderDesignerTestsGroup, AdvancedSelectDisable_changed_network_state) {
    TestableLadder testable;

    auto network = new TestableNetwork(LogicItemState::lisActive);
    testable.Append(network);

    network->SwitchToDisabling();
    testable.HandleButtonSelect();
    CHECK_EQUAL(LogicItemState::lisPassive, network->PublicMorozov_state());

    network->SwitchToDisabling();
    testable.HandleButtonSelect();
    CHECK_EQUAL(LogicItemState::lisActive, network->PublicMorozov_state());
}

TEST(LogicLadderDesignerTestsGroup, RemoveNetworkIfEmpty) {
    TestableLadder testable;

    auto network0 = new Network(LogicItemState::lisActive);
    network0->Append(new InputNC(MapIO::DI));
    network0->Append(new DirectOutput(MapIO::O1));
    testable.Append(network0);

    testable.Append(new Network(LogicItemState::lisActive));

    auto network1 = new Network(LogicItemState::lisActive);
    network1->Append(new InputNC(MapIO::V1));
    network1->Append(new InputNC(MapIO::V2));
    network1->Append(new DirectOutput(MapIO::O2));
    testable.Append(network1);

    testable.Append(new Network(LogicItemState::lisActive));

    CHECK_FALSE(testable.PublicMorozov_RemoveNetworkIfEmpty(0));
    CHECK_FALSE(testable.PublicMorozov_RemoveNetworkIfEmpty(2));
    CHECK_TRUE(testable.PublicMorozov_RemoveNetworkIfEmpty(1));
    CHECK_EQUAL(3, testable.size());
    CHECK_EQUAL(network1, testable[1]);

    CHECK_TRUE(testable.PublicMorozov_RemoveNetworkIfEmpty(2));
    CHECK_EQUAL(2, testable.size());
    CHECK_EQUAL(network1, testable[1]);
}

TEST(LogicLadderDesignerTestsGroup, HandleButtonSelect_removes_empty_network_after_its_editing) {
    TestableLadder testable;

    auto network0 = new Network(LogicItemState::lisActive);
    network0->Append(new InputNC(MapIO::DI));
    network0->Append(new DirectOutput(MapIO::O1));
    testable.Append(network0);
    testable.Append(new Network(LogicItemState::lisActive));

    testable.HandleButtonSelect();
    CHECK_EQUAL(0, testable.PublicMorozov_GetSelectedNetwork());
    testable.HandleButtonDown();
    CHECK_EQUAL(1, testable.PublicMorozov_GetSelectedNetwork());
    testable.HandleButtonSelect();
    CHECK_EQUAL(1, testable.PublicMorozov_GetSelectedNetwork());

    CHECK_EQUAL(EditableElement::ElementState::des_Editing,
                testable.PublicMorozov_GetDesignState(1));

    testable.HandleButtonSelect();

    CHECK_EQUAL(1, testable.size());
}

TEST(LogicLadderDesignerTestsGroup, HandleButtonDown_removes_empty_network_after_its_editing) {
    TestableLadder testable;

    testable.Append(new Network(LogicItemState::lisActive));
    auto network0 = new Network(LogicItemState::lisActive);
    network0->Append(new InputNC(MapIO::DI));
    network0->Append(new DirectOutput(MapIO::O1));
    testable.Append(network0);

    testable.HandleButtonSelect();
    CHECK_EQUAL(0, testable.PublicMorozov_GetSelectedNetwork());
    testable.HandleButtonDown();
    CHECK_EQUAL(0, testable.PublicMorozov_GetSelectedNetwork());

    CHECK_EQUAL(1, testable.size());
}

TEST(LogicLadderDesignerTestsGroup, HandleButtonUp_removes_empty_network_after_its_editing) {
    TestableLadder testable;

    testable.Append(new Network(LogicItemState::lisActive));

    testable.HandleButtonSelect();
    CHECK_EQUAL(0, testable.PublicMorozov_GetSelectedNetwork());
    testable.HandleButtonUp();
    CHECK_EQUAL(-1, testable.PublicMorozov_GetSelectedNetwork());

    CHECK_EQUAL(0, testable.size());
}

TEST(LogicLadderDesignerTestsGroup,
     HandleButtonSelect_immediatelly_creates_new_network_if_there_is_none) {
    TestableLadder testable;

    CHECK_EQUAL(0, testable.size());
    testable.HandleButtonSelect();
    CHECK_EQUAL(1, testable.size());
}

TEST(LogicLadderDesignerTestsGroup, HandleButtonSelect_calls_store_after_network_editing) {
    TestableLadder testable;

    auto network0 = new Network(LogicItemState::lisActive);
    network0->Append(new InputNC(MapIO::DI));
    network0->Append(new DirectOutput(MapIO::O1));
    testable.Append(network0);

    testable.HandleButtonSelect();
    CHECK_EQUAL(0, testable.PublicMorozov_GetSelectedNetwork());
    testable.HandleButtonSelect();
    CHECK_EQUAL(0, testable.PublicMorozov_GetSelectedNetwork());

    CHECK_EQUAL(EditableElement::ElementState::des_Editing,
                testable.PublicMorozov_GetDesignState(0));

    testable.HandleButtonSelect();

    Ladder ladder_load;
    ladder_load.Load();

    CHECK_EQUAL(1, ladder_load.size());
    auto network_load = ladder_load[0];
    CHECK_EQUAL(2, network_load->size());
    CHECK_EQUAL(TvElementType::et_InputNC, (*network_load)[0]->GetElementType());
    CHECK_EQUAL(MapIO::DI, ((InputNC *)(*network_load)[0])->GetIoAdr());
    CHECK_EQUAL(TvElementType::et_DirectOutput, (*network_load)[1]->GetElementType());
    CHECK_EQUAL(MapIO::O1, ((DirectOutput *)(*network_load)[1])->GetIoAdr());
}

TEST(LogicLadderDesignerTestsGroup, SetViewTopIndex_do_nothing_when_incorrect_index) {
    TestableLadder testable;
    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));

    testable.SetViewTopIndex(-1);
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.SetViewTopIndex(3);
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.SetViewTopIndex(4);
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());
}

TEST(LogicLadderDesignerTestsGroup, SetViewTopIndex) {
    TestableLadder testable;
    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));

    testable.SetViewTopIndex(0);
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.SetViewTopIndex(1);
    CHECK_EQUAL(1, *testable.PublicMorozov_Get_view_top_index());

    testable.SetViewTopIndex(2);
    CHECK_EQUAL(2, *testable.PublicMorozov_Get_view_top_index());
}

TEST(LogicLadderDesignerTestsGroup, SetSelectedNetworkIndex_do_nothing_when_incorrect_index) {
    TestableLadder testable;
    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));
    auto selected_network = new Network();
    testable.Append(selected_network);
    selected_network->Select();
    testable.Append(new Network(LogicItemState::lisActive));

    testable.SetSelectedNetworkIndex(-1);
    CHECK_EQUAL(2, testable.PublicMorozov_GetSelectedNetwork());

    testable.SetSelectedNetworkIndex(5);
    CHECK_EQUAL(2, testable.PublicMorozov_GetSelectedNetwork());
}

TEST(LogicLadderDesignerTestsGroup, SetSelectedNetworkIndex_when_no_preselected) {
    TestableLadder testable;
    auto network0 = new Network(LogicItemState::lisActive);
    auto network1 = new Network(LogicItemState::lisActive);
    auto network2 = new Network(LogicItemState::lisActive);
    auto network3 = new Network(LogicItemState::lisActive);
    testable.Append(network0);
    testable.Append(network1);
    testable.Append(network2);
    testable.Append(network3);

    testable.SetViewTopIndex(0);
    testable.SetSelectedNetworkIndex(1);
    CHECK_EQUAL(1, testable.PublicMorozov_GetSelectedNetwork());
    CHECK_TRUE(network1->Selected());

    testable.SetViewTopIndex(2);
    testable.SetSelectedNetworkIndex(3);
    CHECK_EQUAL(3, testable.PublicMorozov_GetSelectedNetwork());
    CHECK_FALSE(network1->Selected());
    CHECK_TRUE(network3->Selected());
}

TEST(LogicLadderDesignerTestsGroup, SetSelectedNetworkIndex_limit_with_ViewTopIndex_range) {
    TestableLadder testable;
    auto network0 = new Network(LogicItemState::lisActive);
    auto network1 = new Network(LogicItemState::lisActive);
    auto network2 = new Network(LogicItemState::lisActive);
    auto network3 = new Network(LogicItemState::lisActive);
    testable.Append(network0);
    testable.Append(network1);
    testable.Append(network2);
    testable.Append(network3);

    testable.SetViewTopIndex(2);
    testable.SetSelectedNetworkIndex(0);
    CHECK_EQUAL(2, testable.PublicMorozov_GetSelectedNetwork());
    CHECK_TRUE(network2->Selected());

    testable.SetViewTopIndex(0);
    testable.SetSelectedNetworkIndex(3);
    CHECK_EQUAL(1, testable.PublicMorozov_GetSelectedNetwork());
    CHECK_FALSE(network2->Selected());
    CHECK_TRUE(network1->Selected());
}

TEST(LogicLadderDesignerTestsGroup, SetSelectedNetworkIndex_if_network_is_editing_then_do_nothing) {
    TestableLadder testable;
    auto network0 = new Network(LogicItemState::lisActive);
    auto network1 = new Network(LogicItemState::lisActive);
    auto network2 = new Network(LogicItemState::lisActive);
    auto network3 = new Network(LogicItemState::lisActive);
    testable.Append(network0);
    testable.Append(network1);
    testable.Append(network2);
    testable.Append(network3);

    testable.SetSelectedNetworkIndex(1);
    CHECK_EQUAL(1, testable.PublicMorozov_GetSelectedNetwork());
    CHECK_TRUE(network1->Selected());
    network1->BeginEditing();

    testable.SetSelectedNetworkIndex(3);
    CHECK_EQUAL(1, testable.PublicMorozov_GetSelectedNetwork());
    CHECK_TRUE(network1->Editing());
    CHECK_FALSE(network3->Selected());
}

TEST(LogicLadderDesignerTestsGroup,
     HandleButtonUp_calls__Controller_UpdateUIViewTop_UpdateUISelected__when_scrolling) {
    TestableLadder testable;
    auto network0 = new Network(LogicItemState::lisActive);
    auto network1 = new Network(LogicItemState::lisActive);
    auto network2 = new Network(LogicItemState::lisActive);
    auto network3 = new Network(LogicItemState::lisActive);
    network0->Append(new InputNC(MapIO::DI));
    network1->Append(new InputNC(MapIO::DI));
    network2->Append(new InputNC(MapIO::DI));
    network3->Append(new InputNC(MapIO::DI));
    testable.Append(network0);
    testable.Append(network1);
    testable.Append(network2);
    testable.Append(network3);

    *testable.PublicMorozov_Get_view_top_index() = 2;

    testable.HandleButtonUp();
    CHECK_EQUAL(1, hotreload->view_top_index);
    CHECK_EQUAL(0, hotreload->selected_network);

    *testable.PublicMorozov_Get_view_top_index() = 3;
    network3->Select();

    testable.HandleButtonUp();
    CHECK_EQUAL(2, *testable.PublicMorozov_Get_view_top_index());
    CHECK_EQUAL(2, hotreload->view_top_index);
    CHECK_EQUAL(2, hotreload->selected_network);

    testable.HandleButtonUp();
    CHECK_EQUAL(1, *testable.PublicMorozov_Get_view_top_index());
    CHECK_EQUAL(1, hotreload->view_top_index);
    CHECK_EQUAL(1, hotreload->selected_network);
}

TEST(LogicLadderDesignerTestsGroup,
     HandleButtonDown_calls__Controller_UpdateUIViewTop_UpdateUISelected__when_scrolling) {
    TestableLadder testable;
    auto network0 = new Network(LogicItemState::lisActive);
    auto network1 = new Network(LogicItemState::lisActive);
    auto network2 = new Network(LogicItemState::lisActive);
    auto network3 = new Network(LogicItemState::lisActive);
    network0->Append(new InputNC(MapIO::DI));
    network1->Append(new InputNC(MapIO::DI));
    network2->Append(new InputNC(MapIO::DI));
    network3->Append(new InputNC(MapIO::DI));
    testable.Append(network0);
    testable.Append(network1);
    testable.Append(network2);
    testable.Append(network3);

    *testable.PublicMorozov_Get_view_top_index() = 1;

    testable.HandleButtonDown();
    CHECK_EQUAL(2, hotreload->view_top_index);
    CHECK_EQUAL(0, hotreload->selected_network);

    *testable.PublicMorozov_Get_view_top_index() = 0;
    network0->Select();

    testable.HandleButtonDown();
    CHECK_EQUAL(2, hotreload->view_top_index);
    CHECK_EQUAL(1, hotreload->selected_network);

    testable.HandleButtonDown();
    CHECK_EQUAL(1, *testable.PublicMorozov_Get_view_top_index());
    CHECK_EQUAL(1, hotreload->view_top_index);
    CHECK_EQUAL(2, hotreload->selected_network);
}

TEST(LogicLadderDesignerTestsGroup, HandleButtonSelect_calls__Controller_UpdateUISelected_changed) {
    TestableLadder testable;

    auto network0 = new Network(LogicItemState::lisActive);
    network0->Append(new InputNC(MapIO::DI));
    testable.Append(network0);
    testable.Append(new Network(LogicItemState::lisActive));

    testable.HandleButtonSelect();
    CHECK_EQUAL(0, testable.PublicMorozov_GetSelectedNetwork());
    CHECK_EQUAL(0, hotreload->view_top_index);
    CHECK_EQUAL(0, hotreload->selected_network);
    CHECK_TRUE(testable[0]->Selected());

    hotreload->selected_network = -1;

    testable.HandleButtonSelect();
    CHECK_EQUAL(0, testable.PublicMorozov_GetSelectedNetwork());
    CHECK_EQUAL(0, hotreload->selected_network);
    CHECK_TRUE(testable[0]->Editing());

    hotreload->selected_network = -1;

    testable.HandleButtonSelect();
    CHECK_EQUAL(-1, testable.PublicMorozov_GetSelectedNetwork());
    CHECK_EQUAL(0, hotreload->selected_network);
    CHECK_FALSE(testable[0]->Editing());
    CHECK_FALSE(testable[0]->Selected());
}

TEST(LogicLadderDesignerTestsGroup, HandleButtonSelect_calls__Controller_DesignStart) {
    Controller::DesignEnd();
    TestableLadder testable;

    auto network0 = new Network(LogicItemState::lisActive);
    network0->Append(new InputNC(MapIO::DI));
    testable.Append(network0);
    testable.Append(new Network(LogicItemState::lisActive));

    CHECK_FALSE(Controller::InDesign());

    testable.HandleButtonSelect();
    CHECK_TRUE(testable[0]->Selected());
    CHECK_TRUE(Controller::InDesign());

    testable.HandleButtonSelect();
    CHECK_TRUE(testable[0]->Editing());
    CHECK_TRUE(Controller::InDesign());

    testable.HandleButtonSelect();
    CHECK_FALSE(testable[0]->Editing());
    CHECK_FALSE(testable[0]->Selected());
    CHECK_FALSE(Controller::InDesign());
}

TEST(LogicLadderDesignerTestsGroup, HandleButtonOption_enter_to_advanced_editing) {
    TestableLadder testable;

    auto network = new TestableNetwork(LogicItemState::lisActive);
    testable.Append(network);
    testable.HandleButtonSelect();

    testable.HandleButtonOption();
    CHECK_TRUE(testable[0]->GetEditable_state()
               == EditableElement::ElementState::des_AdvancedSelectMove);
}

TEST(LogicLadderDesignerTestsGroup, HandleButtonOption_leave_from_advanced_editing) {
    TestableLadder testable;

    auto network = new TestableNetwork(LogicItemState::lisActive);
    testable.Append(network);

    testable[0]->SwitchToAdvancedSelectMove();
    testable.HandleButtonOption();
    CHECK_TRUE(testable[0]->GetEditable_state() == EditableElement::ElementState::des_Regular);

    testable[0]->SwitchToAdvancedSelectCopy();
    testable.HandleButtonOption();
    CHECK_TRUE(testable[0]->GetEditable_state() == EditableElement::ElementState::des_Regular);

    testable[0]->SwitchToAdvancedSelectDelete();
    testable.HandleButtonOption();
    CHECK_TRUE(testable[0]->GetEditable_state() == EditableElement::ElementState::des_Regular);

    testable[0]->SwitchToAdvancedSelectDisable();
    testable.HandleButtonOption();
    CHECK_TRUE(testable[0]->GetEditable_state() == EditableElement::ElementState::des_Regular);

    testable[0]->SwitchToMoving();
    testable.HandleButtonOption();
    CHECK_TRUE(testable[0]->GetEditable_state() == EditableElement::ElementState::des_Regular);

    testable[0]->SwitchToCopying();
    testable.HandleButtonOption();
    CHECK_TRUE(testable[0]->GetEditable_state() == EditableElement::ElementState::des_Regular);

    testable[0]->SwitchToDisabling();
    testable.HandleButtonOption();
    CHECK_TRUE(testable[0]->GetEditable_state() == EditableElement::ElementState::des_Regular);
}

TEST(LogicLadderDesignerTestsGroup, Switch_to_next_advanced_editing_mode) {
    TestableLadder testable;

    auto network = new TestableNetwork(LogicItemState::lisActive);
    testable.Append(network);
    testable.HandleButtonSelect();

    testable.HandleButtonOption();
    CHECK_TRUE(testable[0]->GetEditable_state()
               == EditableElement::ElementState::des_AdvancedSelectMove);

    testable.HandleButtonDown();
    CHECK_TRUE(testable[0]->GetEditable_state()
               == EditableElement::ElementState::des_AdvancedSelectCopy);

    testable.HandleButtonDown();
    CHECK_TRUE(testable[0]->GetEditable_state()
               == EditableElement::ElementState::des_AdvancedSelectDelete);

    testable.HandleButtonDown();
    CHECK_TRUE(testable[0]->GetEditable_state()
               == EditableElement::ElementState::des_AdvancedSelectDisable);

    testable.HandleButtonDown();
    CHECK_TRUE(testable[0]->GetEditable_state()
               == EditableElement::ElementState::des_AdvancedSelectMove);
}

TEST(LogicLadderDesignerTestsGroup, Switch_to_prev_advanced_editing_mode) {
    TestableLadder testable;

    auto network = new TestableNetwork(LogicItemState::lisActive);
    testable.Append(network);
    testable.HandleButtonSelect();

    testable.HandleButtonOption();
    CHECK_TRUE(testable[0]->GetEditable_state()
               == EditableElement::ElementState::des_AdvancedSelectMove);

    testable.HandleButtonUp();
    CHECK_TRUE(testable[0]->GetEditable_state()
               == EditableElement::ElementState::des_AdvancedSelectDisable);

    testable.HandleButtonUp();
    CHECK_TRUE(testable[0]->GetEditable_state()
               == EditableElement::ElementState::des_AdvancedSelectDelete);

    testable.HandleButtonUp();
    CHECK_TRUE(testable[0]->GetEditable_state()
               == EditableElement::ElementState::des_AdvancedSelectCopy);

    testable.HandleButtonUp();
    CHECK_TRUE(testable[0]->GetEditable_state()
               == EditableElement::ElementState::des_AdvancedSelectMove);
}

TEST(LogicLadderDesignerTestsGroup, HandleButtonSelect_switch_to_move_network_mode) {
    TestableLadder testable;

    auto network = new TestableNetwork(LogicItemState::lisActive);
    testable.Append(network);

    testable[0]->SwitchToAdvancedSelectMove();

    testable.HandleButtonSelect();
    CHECK_TRUE(testable[0]->GetEditable_state() == EditableElement::ElementState::des_Moving);
}

TEST(LogicLadderDesignerTestsGroup, HandleButtonSelect_switch_to_copy_network_mode) {
    TestableLadder testable;

    auto network = new TestableNetwork(LogicItemState::lisActive);
    testable.Append(network);

    testable[0]->SwitchToAdvancedSelectCopy();

    testable.HandleButtonSelect();
    CHECK_TRUE(testable[0]->GetEditable_state() == EditableElement::ElementState::des_Copying);
}

TEST(LogicLadderDesignerTestsGroup, HandleButtonSelect_switch_to_delete_network_mode) {
    TestableLadder testable;

    auto network = new TestableNetwork(LogicItemState::lisActive);
    testable.Append(network);

    testable[0]->SwitchToAdvancedSelectDelete();

    testable.HandleButtonSelect();
    CHECK_TRUE(testable[0]->GetEditable_state() == EditableElement::ElementState::des_Deleting);
}

TEST(LogicLadderDesignerTestsGroup, HandleButtonSelect_switch_to_disable_network_mode) {
    TestableLadder testable;

    auto network = new TestableNetwork(LogicItemState::lisActive);
    testable.Append(network);

    testable[0]->SwitchToAdvancedSelectDisable();

    testable.HandleButtonSelect();
    CHECK_TRUE(testable[0]->GetEditable_state() == EditableElement::ElementState::des_Disabling);
}

TEST(LogicLadderDesignerTestsGroup, HandleButtonDown_move_network_to_up) {
    TestableLadder testable;
    auto network0 = new Network(LogicItemState::lisActive);
    auto network1 = new Network(LogicItemState::lisActive);
    auto network2 = new Network(LogicItemState::lisActive);
    auto network3 = new Network(LogicItemState::lisActive);
    network0->Append(new InputNC(MapIO::DI));
    network1->Append(new InputNC(MapIO::DI));
    network2->Append(new InputNC(MapIO::DI));
    network3->Append(new InputNC(MapIO::DI));
    testable.Append(network0);
    testable.Append(network1);
    testable.Append(network2);
    testable.Append(network3);

    *testable.PublicMorozov_Get_view_top_index() = 3;

    testable[3]->SwitchToMoving();
    CHECK_TRUE(testable[3]->GetEditable_state() == EditableElement::ElementState::des_Moving);

    testable.HandleButtonUp();
    CHECK_EQUAL(testable[2], network3);
    CHECK_EQUAL(testable[3], network2);
    CHECK_EQUAL(2, *testable.PublicMorozov_Get_view_top_index());

    testable.HandleButtonUp();
    CHECK_EQUAL(testable[1], network3);
    CHECK_EQUAL(testable[2], network1);
    CHECK_EQUAL(1, *testable.PublicMorozov_Get_view_top_index());

    testable.HandleButtonUp();
    CHECK_EQUAL(testable[0], network3);
    CHECK_EQUAL(testable[1], network0);
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.HandleButtonUp();
    CHECK_EQUAL(testable[0], network3);
    CHECK_EQUAL(testable[1], network0);
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());
}

TEST(LogicLadderDesignerTestsGroup, HandleButtonDown_move_network_to_down) {
    TestableLadder testable;
    auto network0 = new Network(LogicItemState::lisActive);
    auto network1 = new Network(LogicItemState::lisActive);
    auto network2 = new Network(LogicItemState::lisActive);
    auto network3 = new Network(LogicItemState::lisActive);
    network0->Append(new InputNC(MapIO::DI));
    network1->Append(new InputNC(MapIO::DI));
    network2->Append(new InputNC(MapIO::DI));
    network3->Append(new InputNC(MapIO::DI));
    testable.Append(network0);
    testable.Append(network1);
    testable.Append(network2);
    testable.Append(network3);

    *testable.PublicMorozov_Get_view_top_index() = 0;

    testable[0]->SwitchToMoving();
    CHECK_TRUE(testable[0]->GetEditable_state() == EditableElement::ElementState::des_Moving);

    testable.HandleButtonDown();
    CHECK_EQUAL(testable[0], network1);
    CHECK_EQUAL(testable[1], network0);
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.HandleButtonDown();
    CHECK_EQUAL(testable[1], network2);
    CHECK_EQUAL(testable[2], network0);
    CHECK_EQUAL(1, *testable.PublicMorozov_Get_view_top_index());

    testable.HandleButtonDown();
    CHECK_EQUAL(testable[2], network3);
    CHECK_EQUAL(testable[3], network0);
    CHECK_EQUAL(2, *testable.PublicMorozov_Get_view_top_index());

    testable.HandleButtonDown();
    CHECK_EQUAL(testable[2], network3);
    CHECK_EQUAL(testable[3], network0);
    CHECK_EQUAL(3, *testable.PublicMorozov_Get_view_top_index());

    testable.HandleButtonDown();
    CHECK_EQUAL(testable[2], network3);
    CHECK_EQUAL(testable[3], network0);
    CHECK_EQUAL(3, *testable.PublicMorozov_Get_view_top_index());
}

TEST(LogicLadderDesignerTestsGroup, AdvancedEditing__duplicate_network) {
    TestableLadder testable;
    auto network0 = new Network(LogicItemState::lisActive);
    network0->Append(new InputNC(MapIO::DI));
    testable.Append(network0);

    testable[0]->SwitchToCopying();
    CHECK_TRUE(testable[0]->GetEditable_state() == EditableElement::ElementState::des_Copying);

    testable.HandleButtonSelect();
    CHECK_EQUAL(2, testable.size());
    CHECK_EQUAL(testable[1], network0);
    CHECK_EQUAL(1, testable[0]->size());
    CHECK_EQUAL(TvElementType::et_InputNC, testable[0]->at(0)->GetElementType());
}

TEST(LogicLadderDesignerTestsGroup, AdvancedEditing__delete_network) {
    TestableLadder testable;
    auto network0 = new Network(LogicItemState::lisActive);
    auto network1 = new Network(LogicItemState::lisActive);
    auto network2 = new Network(LogicItemState::lisActive);
    network0->Append(new InputNC(MapIO::DI));
    network1->Append(new InputNC(MapIO::DI));
    network2->Append(new InputNC(MapIO::DI));
    testable.Append(network0);
    testable.Append(network1);
    testable.Append(network2);

    testable[0]->SwitchToDeleting();
    CHECK_TRUE(testable[0]->GetEditable_state() == EditableElement::ElementState::des_Deleting);

    testable.HandleButtonSelect();
    CHECK_EQUAL(2, testable.size());
    CHECK_EQUAL(testable[0], network1);
}

TEST(LogicLadderDesignerTestsGroup, AdvancedEditing__disable_network) {
    TestableLadder testable;
    auto network0 = new Network(LogicItemState::lisActive);
    network0->Append(new InputNC(MapIO::DI));
    testable.Append(network0);

    testable[0]->SwitchToDisabling();
    CHECK_TRUE(testable[0]->GetEditable_state() == EditableElement::ElementState::des_Disabling);
    CHECK_EQUAL(LogicItemState::lisActive, testable[0]->GetState());

    testable.HandleButtonSelect();
    CHECK_EQUAL(LogicItemState::lisPassive, testable[0]->GetState());
}
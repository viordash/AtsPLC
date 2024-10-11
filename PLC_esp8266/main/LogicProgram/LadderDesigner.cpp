#include "Display/ScrollBar.h"
#include "LogicProgram/Ladder.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool Ladder::CanScrollAuto() {
    return (size_t)view_top_index == size() - Ladder::MaxViewPortCount;
}

void Ladder::AutoScroll() {
    if (size() > Ladder::MaxViewPortCount) {
        view_top_index = size() - Ladder::MaxViewPortCount;
    }
}

int Ladder::GetSelectedNetwork() {
    for (int i = 0; i < (int)size(); i++) {
        auto network = (*this)[i];
        if (network->Selected() || network->Editing()) {
            return i;
        }
    }
    return -1;
}

TEditableElementState Ladder::GetDesignState(int selected_network) {
    if (selected_network < 0) {
        return TEditableElementState::des_Regular;
    }

    auto network = (*this)[selected_network];
    if (network->Editing()) {
        return TEditableElementState::des_Editing;
    }
    if (network->Selected()) {
        return TEditableElementState::des_Selected;
    }
    ESP_LOGE(TAG_Ladder, "GetDesignState, unexpected network (id:%d) state", selected_network);
    return TEditableElementState::des_Regular;
}

void Ladder::HandleButtonUp() {
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGD(TAG_Ladder,
             "HandleButtonUp, %u, view_top_index:%u, selected_network:%d",
             (unsigned)design_state,
             view_top_index,
             selected_network);

    switch (design_state) {
        case TEditableElementState::des_Regular:
            if (view_top_index > 0) {
                view_top_index--;
            }
            break;

        case TEditableElementState::des_Selected:
            (*this)[selected_network]->CancelSelection();

            if (selected_network > view_top_index) {
                selected_network--;
            } else if (view_top_index > 0) {
                view_top_index--;
                selected_network--;
            }

            (*this)[selected_network]->Select();
            break;

        case TEditableElementState::des_Editing:
            (*this)[selected_network]->HandleButtonUp();
            return;
            
        case TEditableElementState::des_Editing_Property_0:
        case TEditableElementState::des_Editing_Property_1:
            break;
    }
}

void Ladder::HandleButtonDown() {
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGD(TAG_Ladder,
             "HandleButtonDown, %u, view_top_index:%u, selected_network:%d",
             (unsigned)design_state,
             view_top_index,
             selected_network);

    switch (design_state) {
        case TEditableElementState::des_Regular:
            if (view_top_index + Ladder::MaxViewPortCount < size()) {
                view_top_index++;
            }
            break;

        case TEditableElementState::des_Selected:
            (*this)[selected_network]->CancelSelection();

            if (selected_network == view_top_index) {
                selected_network++;
            } else if (view_top_index + Ladder::MaxViewPortCount < size()) {
                view_top_index++;
                selected_network++;
            }

            (*this)[selected_network]->Select();
            break;

        case TEditableElementState::des_Editing: {
            auto network = (*this)[selected_network];
            network->HandleButtonDown();
            return;
        }

        case TEditableElementState::des_Editing_Property_0:
        case TEditableElementState::des_Editing_Property_1:
            break;
    }
}

void Ladder::HandleButtonSelect() {
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGD(TAG_Ladder,
             "HandleButtonSelect, %u, view_top_index:%u, selected_network:%d",
             (unsigned)design_state,
             (unsigned)view_top_index,
             selected_network);

    switch (design_state) {
        case TEditableElementState::des_Regular:
            (*this)[view_top_index]->Select();
            break;

        case TEditableElementState::des_Selected:
            (*this)[selected_network]->CancelSelection();
            break;

        case TEditableElementState::des_Editing:
            (*this)[selected_network]->HandleButtonSelect();
            return;

        default:
            break;
    }
}

void Ladder::HandleButtonOption() {
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGD(TAG_Ladder,
             "SwitchEditing, %u, selected_network:%d",
             (unsigned)design_state,
             selected_network);
    switch (design_state) {
        case TEditableElementState::des_Selected:
            (*this)[selected_network]->BeginEditing();
            break;
        case TEditableElementState::des_Editing:
            (*this)[selected_network]->HandleButtonOption();
            break;

        default:
            ESP_LOGE(TAG_Ladder,
                     "SwitchEditing, unexpected network (id:%d) state (%u)",
                     selected_network,
                     (unsigned)design_state);
            break;
    }
}
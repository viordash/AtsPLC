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
        if (network->GetDesignState() != TEditableElementState::des_Regular) {
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
    auto network_state = network->GetDesignState();
    switch (network_state) {
        case TEditableElementState::des_Editing:
            return TEditableElementState::des_Editing;

        case TEditableElementState::des_Selected:
            return TEditableElementState::des_Selected;
        default:
            ESP_LOGE(TAG_Ladder,
                     "GetDesignState, unexpected network (id:%d) state (%u)",
                     selected_network,
                     (unsigned)network_state);
            break;
    }
    return TEditableElementState::des_Regular;
}

void Ladder::UpdateDesigning(TEditableElementState design_state, int selected_network) {
    for (int i = 0; i < (int)size(); i++) {
        auto network = (*this)[i];
        network->ChangeSelection(design_state == TEditableElementState::des_Selected
                                 && i == selected_network);
        network->ChangeEditing(design_state == TEditableElementState::des_Editing
                               && i == selected_network);
    }
}

void Ladder::ScrollUp() {
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGI(TAG_Ladder,
             "ScrollUp, %u, view_top_index:%u, selected_network:%d",
             (unsigned)design_state,
             view_top_index,
             selected_network);

    switch (design_state) {
        case TEditableElementState::des_Regular:
            if (view_top_index > 0) {
                view_top_index--;
                selected_network--;
            }
            UpdateDesigning(design_state, selected_network);
            break;

        case TEditableElementState::des_Selected:
            if (selected_network > view_top_index) {
                selected_network--;
            } else if (view_top_index > 0) {
                view_top_index--;
                selected_network--;
            }
            UpdateDesigning(design_state, selected_network);
            break;

        case TEditableElementState::des_Editing: {
            auto network = (*this)[selected_network];
            network->ScrollUp();
            return;
        }
    }
}

void Ladder::ScrollDown() {
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGI(TAG_Ladder,
             "ScrollDown, %u, view_top_index:%u, selected_network:%d",
             (unsigned)design_state,
             view_top_index,
             selected_network);

    switch (design_state) {
        case TEditableElementState::des_Regular:
            if (view_top_index + Ladder::MaxViewPortCount < size()) {
                view_top_index++;
                selected_network++;
            }
            UpdateDesigning(design_state, selected_network);
            break;

        case TEditableElementState::des_Selected:
            if (selected_network == view_top_index) {
                selected_network++;
            } else if (view_top_index + Ladder::MaxViewPortCount < size()) {
                view_top_index++;
                selected_network++;
            }
            UpdateDesigning(design_state, selected_network);
            break;

        case TEditableElementState::des_Editing: {
            auto network = (*this)[selected_network];
            network->ScrollDown();
            return;
        }
    }
}

void Ladder::SwitchSelecting() {
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGI(TAG_Ladder,
             "SwitchSelecting, %u, view_top_index:%u, selected_network:%d",
             (unsigned)design_state,
             (unsigned)view_top_index,
             selected_network);

    switch (design_state) {
        case TEditableElementState::des_Regular:
            design_state = TEditableElementState::des_Selected;
            selected_network = view_top_index;
            UpdateDesigning(design_state, selected_network);
            break;

        case TEditableElementState::des_Selected:
            design_state = TEditableElementState::des_Regular;
            selected_network = -1;
            UpdateDesigning(design_state, selected_network);
            break;

        case TEditableElementState::des_Editing: {
            auto network = (*this)[selected_network];
            network->SwitchSelecting();
            return;
        }

        default:
            break;
    }
}

void Ladder::SwitchEditing() {
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGI(TAG_Ladder, "SwitchEditing, %u", (unsigned)design_state);
    switch (design_state) {
        case TEditableElementState::des_Selected:
            design_state = TEditableElementState::des_Editing;
            (*this)[selected_network]->BeginEditing();
            break;
        case TEditableElementState::des_Editing:
            design_state = TEditableElementState::des_Regular;
            (*this)[selected_network]->EndEditing();
            break;

        default:
            break;
    }

    UpdateDesigning(design_state, selected_network);
}
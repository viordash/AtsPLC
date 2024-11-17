#include "Display/ScrollBar.h"
#include "LogicProgram/Ladder.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
extern "C" {
#include "hotreload_service.h"
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool Ladder::CanScrollAuto() {
    return (size_t)hotreload->view_top_index == size() - Ladder::MaxViewPortCount;
}

void Ladder::AutoScroll() {
    if (size() > Ladder::MaxViewPortCount) {
        hotreload->view_top_index = size() - Ladder::MaxViewPortCount;
        store_hotreload();
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

EditableElement::ElementState Ladder::GetDesignState(int selected_network) {
    if (selected_network < 0) {
        return EditableElement::ElementState::des_Regular;
    }

    auto network = (*this)[selected_network];
    if (network->Editing()) {
        return EditableElement::ElementState::des_Editing;
    }
    if (network->Selected()) {
        return EditableElement::ElementState::des_Selected;
    }
    ESP_LOGE(TAG_Ladder, "GetDesignState, unexpected network (id:%d) state", selected_network);
    return EditableElement::ElementState::des_Regular;
}

void Ladder::HandleButtonUp() {
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGD(TAG_Ladder,
             "HandleButtonUp, %u, view_top_index:%u, selected_network:%d",
             (unsigned)design_state,
             hotreload->view_top_index,
             selected_network);

    switch (design_state) {
        case EditableElement::ElementState::des_Regular:
            if (hotreload->view_top_index > 0) {
                hotreload->view_top_index--;
                store_hotreload();
            }
            break;

        case EditableElement::ElementState::des_Selected:
            (*this)[selected_network]->CancelSelection();
            RemoveNetworkIfEmpty(selected_network);

            if (selected_network > hotreload->view_top_index) {
                selected_network--;
            } else if (hotreload->view_top_index > 0) {
                hotreload->view_top_index--;
                store_hotreload();
                selected_network--;
            }
            if (size() > 0) {
                (*this)[selected_network]->Select();
            }
            break;

        case EditableElement::ElementState::des_Editing:
            (*this)[selected_network]->SelectPrior();
            return;
    }
}

void Ladder::HandleButtonPageUp() {
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGI(TAG_Ladder,
             "HandleButtonPageUp, %u, view_top_index:%u, selected_network:%d",
             (unsigned)design_state,
             hotreload->view_top_index,
             selected_network);

    switch (design_state) {
        case EditableElement::ElementState::des_Editing:
            (*this)[selected_network]->PageUp();
            return;
        default:
            HandleButtonUp();
            break;
    }
}

void Ladder::HandleButtonDown() {
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGD(TAG_Ladder,
             "HandleButtonDown, %u, view_top_index:%u, selected_network:%d",
             (unsigned)design_state,
             hotreload->view_top_index,
             selected_network);

    switch (design_state) {
        case EditableElement::ElementState::des_Regular:
            if (hotreload->view_top_index + Ladder::MaxViewPortCount < size()) {
                hotreload->view_top_index++;
                store_hotreload();
            }
            break;

        case EditableElement::ElementState::des_Selected:
            (*this)[selected_network]->CancelSelection();

            if (!RemoveNetworkIfEmpty(selected_network)) {
                if (selected_network == hotreload->view_top_index) {
                    selected_network++;
                } else if (hotreload->view_top_index + Ladder::MaxViewPortCount <= size()) {
                    hotreload->view_top_index++;
                    store_hotreload();
                    selected_network++;
                }
            }

            if (selected_network == (int)size()) {
                auto new_network = new Network(LogicItemState::lisActive);
                Append(new_network);
            }

            (*this)[selected_network]->Select();
            break;

        case EditableElement::ElementState::des_Editing: {
            auto network = (*this)[selected_network];
            network->SelectNext();
            return;
        }
    }
}

void Ladder::HandleButtonPageDown() {
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGI(TAG_Ladder,
             "HandleButtonPageDown, %u, view_top_index:%u, selected_network:%d",
             (unsigned)design_state,
             hotreload->view_top_index,
             selected_network);

    switch (design_state) {
        case EditableElement::ElementState::des_Editing:
            (*this)[selected_network]->PageDown();
            return;
        default:
            HandleButtonDown();
            break;
    }
}

void Ladder::HandleButtonSelect() {
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGD(TAG_Ladder,
             "HandleButtonSelect, %u, view_top_index:%u, selected_network:%d",
             (unsigned)design_state,
             (unsigned)hotreload->view_top_index,
             selected_network);

    switch (design_state) {
        case EditableElement::ElementState::des_Regular:
            if (size() == 0) {
                auto new_network = new Network(LogicItemState::lisActive);
                Append(new_network);
            }
            (*this)[hotreload->view_top_index]->Select();
            break;

        case EditableElement::ElementState::des_Selected:
            (*this)[selected_network]->BeginEditing();
            break;

        case EditableElement::ElementState::des_Editing:
            (*this)[selected_network]->Change();
            if (!(*this)[selected_network]->Editing()) {
                RemoveNetworkIfEmpty(selected_network);
                Store();
            }
            return;

        default:
            break;
    }
}

void Ladder::HandleButtonOption() {
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGD(TAG_Ladder,
             "HandleButtonOption, %u, selected_network:%d",
             (unsigned)design_state,
             selected_network);
    switch (design_state) {
        case EditableElement::ElementState::des_Editing:
            (*this)[selected_network]->SwitchState();
            break;

        default:
            break;
    }
}

bool Ladder::RemoveNetworkIfEmpty(int network_id) {
    auto network = (*this)[network_id];
    if (network->empty()) {
        for (auto it = begin(); it != end(); ++it) {
            if (network == *it) {
                erase(it);
                ESP_LOGI(TAG_Ladder, "delete network: %p", network);
                delete network;
                return true;
            }
        }
    }
    return false;
}

bool Ladder::ForcePeriodicRendering() {
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    switch (design_state) {
        case EditableElement::ElementState::des_Selected:
        case EditableElement::ElementState::des_Editing:
            return true;

        default:
            return false;
    }
}
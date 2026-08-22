#include "Display/ScrollBar.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/Ladder.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int Ladder::GetSelectedNetwork() {
    for (int i = 0; i < (int)items.size(); i++) {
        auto network = items[i];
        switch (network->GetEditable_state()) {
            case EditableElement::ElementState::des_Selected:
            case EditableElement::ElementState::des_Editing:
            case EditableElement::ElementState::des_AdvancedSelectMove:
            case EditableElement::ElementState::des_AdvancedSelectCopy:
            case EditableElement::ElementState::des_AdvancedSelectDelete:
            case EditableElement::ElementState::des_AdvancedSelectDisable:
            case EditableElement::ElementState::des_Moving:
            case EditableElement::ElementState::des_Copying:
            case EditableElement::ElementState::des_Deleting:
            case EditableElement::ElementState::des_Disabling:
                return i;

            default:
                break;
        }
    }
    return -1;
}

EditableElement::ElementState Ladder::GetDesignState(int selected_network) {
    if (selected_network < 0) {
        return EditableElement::ElementState::des_Regular;
    }

    auto network = items[selected_network];
    switch (network->GetEditable_state()) {
        case EditableElement::ElementState::des_Selected:
        case EditableElement::ElementState::des_Editing:
        case EditableElement::ElementState::des_AdvancedSelectMove:
        case EditableElement::ElementState::des_AdvancedSelectCopy:
        case EditableElement::ElementState::des_AdvancedSelectDelete:
        case EditableElement::ElementState::des_AdvancedSelectDisable:
        case EditableElement::ElementState::des_Moving:
        case EditableElement::ElementState::des_Copying:
        case EditableElement::ElementState::des_Deleting:
        case EditableElement::ElementState::des_Disabling:
            return network->GetEditable_state();

        default:
            break;
    }

    ESP_LOGE(TAG_Ladder, "GetDesignState, unexpected network (id:%d) state", selected_network);
    return EditableElement::ElementState::des_Regular;
}

bool Ladder::ScrollUp(int *selected_network) {
    if (*selected_network > view_top_index) {
        (*selected_network)--;
        Controller::UpdateUISelected(*selected_network);
    } else if (view_top_index > 0) {
        view_top_index--;
        (*selected_network)--;
        Controller::UpdateUIViewTop(view_top_index);
        Controller::UpdateUISelected(*selected_network);
    }
    return items.size() > 0;
}

bool Ladder::ScrollDown(int *selected_network) {
    if (*selected_network + 1 < view_top_index + (int)Ladder::MaxViewPortCount) {
        (*selected_network)++;
        Controller::UpdateUISelected(*selected_network);
    } else if (view_top_index + Ladder::MaxViewPortCount <= items.size()) {
        view_top_index++;
        (*selected_network)++;
        Controller::UpdateUIViewTop(view_top_index);
        Controller::UpdateUISelected(*selected_network);
    }
    return *selected_network < (int)items.size();
}

void Ladder::HandleButtonUp() {
    frame_buffer_req_render = true;
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGD(TAG_Ladder,
             "HandleButtonUp, %u, view_top_index:%u, selected_network:%d",
             (unsigned)design_state,
             view_top_index,
             selected_network);

    switch (design_state) {
        case EditableElement::ElementState::des_Regular:
            if (view_top_index > 0) {
                view_top_index--;
                Controller::UpdateUIViewTop(view_top_index);
            }
            break;

        case EditableElement::ElementState::des_Selected:
            items[selected_network]->CancelSelection();
            RemoveNetworkIfEmpty(selected_network);
            if (ScrollUp(&selected_network)) {
                items[selected_network]->Select();
            }
            break;

        case EditableElement::ElementState::des_Editing:
            items[selected_network]->SelectPrior();
            break;

        case EditableElement::ElementState::des_AdvancedSelectMove:
            items[selected_network]->SwitchToAdvancedSelectDisable();
            break;

        case EditableElement::ElementState::des_AdvancedSelectCopy:
            items[selected_network]->SwitchToAdvancedSelectMove();
            break;

        case EditableElement::ElementState::des_AdvancedSelectDelete:
            items[selected_network]->SwitchToAdvancedSelectCopy();
            break;

        case EditableElement::ElementState::des_AdvancedSelectDisable:
            items[selected_network]->SwitchToAdvancedSelectDelete();
            break;

        case EditableElement::ElementState::des_Moving:
            if (selected_network > 0) {
                std::swap(items.at(selected_network), items.at(selected_network - 1));
            }
            ScrollUp(&selected_network);
            break;

        case EditableElement::ElementState::des_Copying:
            break;

        case EditableElement::ElementState::des_Deleting:
            break;

        case EditableElement::ElementState::des_Disabling:
            break;
    }
}

void Ladder::HandleButtonPageUp() {
    frame_buffer_req_render = true;
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGI(TAG_Ladder,
             "HandleButtonPageUp, %u, view_top_index:%u, selected_network:%d",
             (unsigned)design_state,
             view_top_index,
             selected_network);

    switch (design_state) {
        case EditableElement::ElementState::des_Editing:
            items[selected_network]->PageUp();
            return;
        default:
            HandleButtonUp();
            break;
    }
}

void Ladder::HandleButtonDown() {
    frame_buffer_req_render = true;
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGD(TAG_Ladder,
             "HandleButtonDown, %u, view_top_index:%u, selected_network:%d",
             (unsigned)design_state,
             view_top_index,
             selected_network);

    switch (design_state) {
        case EditableElement::ElementState::des_Regular:
            if (view_top_index + Ladder::MaxViewPortCount < items.size()) {
                view_top_index++;
                Controller::UpdateUIViewTop(view_top_index);
            }
            break;

        case EditableElement::ElementState::des_Selected:
            items[selected_network]->CancelSelection();

            if (!RemoveNetworkIfEmpty(selected_network)) {
                ScrollDown(&selected_network);
            }

            if (selected_network == (int)items.size()) {
                auto new_network = work_mode == WorkMode::Stop
                                     ? new Network(NetworkState::nsStopFromActive)
                                     : new Network(NetworkState::nsActive);
                Append(new_network);
            }

            items[selected_network]->Select();
            break;

        case EditableElement::ElementState::des_Editing:
            items[selected_network]->SelectNext();
            break;

        case EditableElement::ElementState::des_AdvancedSelectMove:
            items[selected_network]->SwitchToAdvancedSelectCopy();
            break;

        case EditableElement::ElementState::des_AdvancedSelectCopy:
            items[selected_network]->SwitchToAdvancedSelectDelete();
            break;

        case EditableElement::ElementState::des_AdvancedSelectDelete:
            items[selected_network]->SwitchToAdvancedSelectDisable();
            break;

        case EditableElement::ElementState::des_AdvancedSelectDisable:
            items[selected_network]->SwitchToAdvancedSelectMove();
            break;

        case EditableElement::ElementState::des_Moving:
            if (selected_network + 1 < (int)items.size()) {
                std::swap(items.at(selected_network), items.at(selected_network + 1));
            }
            ScrollDown(&selected_network);
            break;

        case EditableElement::ElementState::des_Copying:
            break;

        case EditableElement::ElementState::des_Deleting:
            break;

        case EditableElement::ElementState::des_Disabling:
            break;
    }
}

void Ladder::HandleButtonPageDown() {
    frame_buffer_req_render = true;
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGI(TAG_Ladder,
             "HandleButtonPageDown, %u, view_top_index:%u, selected_network:%d",
             (unsigned)design_state,
             view_top_index,
             selected_network);

    switch (design_state) {
        case EditableElement::ElementState::des_Editing:
            items[selected_network]->PageDown();
            return;
        default:
            HandleButtonDown();
            break;
    }
}

void Ladder::HandleButtonSelect() {
    frame_buffer_req_render = true;
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGI(TAG_Ladder,
             "HandleButtonSelect, %u, view_top_index:%u, selected_network:%d",
             (unsigned)design_state,
             (unsigned)view_top_index,
             selected_network);

    switch (design_state) {
        case EditableElement::ElementState::des_Regular: {
            if (items.size() == 0) {
                auto new_network = work_mode == WorkMode::Stop
                                     ? new Network(NetworkState::nsStopFromActive)
                                     : new Network(NetworkState::nsActive);
                Append(new_network);
            }

            int32_t last_selected_network = Controller::GetLastUpdatedUISelected();
            last_selected_network =
                std::clamp(last_selected_network,
                           view_top_index,
                           (view_top_index + (int)Ladder::MaxViewPortCount) - 1);

            if (last_selected_network >= 0 && last_selected_network < (int)items.size()) {
                items[last_selected_network]->Select();
            } else {
                items[view_top_index]->Select();
                Controller::UpdateUISelected(view_top_index);
            }
            Controller::DesignStart();
            break;
        }

        case EditableElement::ElementState::des_Selected:
            items[selected_network]->BeginEditing();
            Controller::UpdateUISelected(selected_network);
            break;

        case EditableElement::ElementState::des_Editing:
            items[selected_network]->Change();
            if (!items[selected_network]->Editing()) {
                if (RemoveNetworkIfEmpty(selected_network)) {
                    selected_network = -1;
                }
                Store();
                Controller::UpdateUISelected(selected_network);
                Controller::DesignEnd();
            }
            return;

        case EditableElement::ElementState::des_AdvancedSelectMove:
            items[selected_network]->SwitchToMoving();
            break;

        case EditableElement::ElementState::des_AdvancedSelectCopy:
            items[selected_network]->SwitchToCopying();
            break;

        case EditableElement::ElementState::des_AdvancedSelectDelete:
            items[selected_network]->SwitchToDeleting();
            break;

        case EditableElement::ElementState::des_AdvancedSelectDisable:
            items[selected_network]->SwitchToDisabling();
            break;

        case EditableElement::ElementState::des_Moving:
            items[selected_network]->EndEditing();
            Store();
            Controller::DesignEnd();
            break;

        case EditableElement::ElementState::des_Copying:
            items[selected_network]->EndEditing();
            Duplicate(selected_network);
            Store();
            Controller::DesignEnd();
            break;

        case EditableElement::ElementState::des_Deleting:
            items[selected_network]->EndEditing();
            Delete(selected_network);
            Store();
            Controller::DesignEnd();
            break;

        case EditableElement::ElementState::des_Disabling:
            items[selected_network]->EndEditing();
            items[selected_network]->SwitchState();
            Store();
            Controller::DesignEnd();
            break;

        default:
            break;
    }
}

void Ladder::HandleButtonOption() {
    frame_buffer_req_render = true;
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGI(TAG_Ladder,
             "HandleButtonOption, %u, selected_network:%d",
             (unsigned)design_state,
             selected_network);
    switch (design_state) {
        case EditableElement::ElementState::des_Editing:
            items[selected_network]->Option();
            break;

        case EditableElement::ElementState::des_Selected:
            items[selected_network]->SwitchToAdvancedSelectMove();
            break;

        case EditableElement::ElementState::des_AdvancedSelectMove:
            items[selected_network]->EndEditing();
            Controller::DesignEnd();
            break;

        case EditableElement::ElementState::des_AdvancedSelectCopy:
            items[selected_network]->EndEditing();
            Controller::DesignEnd();
            break;

        case EditableElement::ElementState::des_AdvancedSelectDelete:
            items[selected_network]->EndEditing();
            Controller::DesignEnd();
            break;

        case EditableElement::ElementState::des_AdvancedSelectDisable:
            items[selected_network]->EndEditing();
            Controller::DesignEnd();
            break;

        case EditableElement::ElementState::des_Moving:
            items[selected_network]->EndEditing();
            Controller::DesignEnd();
            break;

        case EditableElement::ElementState::des_Copying:
            items[selected_network]->EndEditing();
            Controller::DesignEnd();
            break;

        case EditableElement::ElementState::des_Deleting:
            items[selected_network]->EndEditing();
            Controller::DesignEnd();
            break;

        case EditableElement::ElementState::des_Disabling:
            items[selected_network]->EndEditing();
            Controller::DesignEnd();
            break;

        default:
            break;
    }
}

bool Ladder::RemoveNetworkIfEmpty(int network_id) {
    auto network = items[network_id];
    if (network->Empty()) {
        for (auto it = items.begin(); it != items.end(); ++it) {
            if (network == *it) {
                items.erase(it);
                ESP_LOGI(TAG_Ladder, "delete network: %p", network);
                delete network;
                return true;
            }
        }
    }
    return false;
}
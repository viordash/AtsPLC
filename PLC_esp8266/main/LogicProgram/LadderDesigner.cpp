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

    auto network = (*this)[selected_network];
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
        Controller::UpdateUISelected(*selected_network);
    }
    return size() > 0;
}

bool Ladder::ScrollDown(int *selected_network) {
    if (*selected_network + 1 < view_top_index + (int)Ladder::MaxViewPortCount) {
        (*selected_network)++;
        Controller::UpdateUISelected(*selected_network);
    } else if (view_top_index + Ladder::MaxViewPortCount <= size()) {
        view_top_index++;
        (*selected_network)++;
        Controller::UpdateUISelected(*selected_network);
    }
    return *selected_network < (int)size();
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
        case EditableElement::ElementState::des_Regular:
            if (view_top_index > 0) {
                view_top_index--;
                Controller::UpdateUIViewTop(view_top_index);
            }
            break;

        case EditableElement::ElementState::des_Selected:
            (*this)[selected_network]->CancelSelection();
            RemoveNetworkIfEmpty(selected_network);
            if (ScrollUp(&selected_network)) {
                (*this)[selected_network]->Select();
            }
            break;

        case EditableElement::ElementState::des_Editing:
            (*this)[selected_network]->SelectPrior();
            break;

        case EditableElement::ElementState::des_AdvancedSelectMove:
            (*this)[selected_network]->SwitchToAdvancedSelectDisable();
            break;

        case EditableElement::ElementState::des_AdvancedSelectCopy:
            (*this)[selected_network]->SwitchToAdvancedSelectMove();
            break;

        case EditableElement::ElementState::des_AdvancedSelectDelete:
            (*this)[selected_network]->SwitchToAdvancedSelectCopy();
            break;

        case EditableElement::ElementState::des_AdvancedSelectDisable:
            (*this)[selected_network]->SwitchToAdvancedSelectDelete();
            break;

        case EditableElement::ElementState::des_Moving:
            if (selected_network > 0) {
                std::swap(at(selected_network), at(selected_network - 1));
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
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGI(TAG_Ladder,
             "HandleButtonPageUp, %u, view_top_index:%u, selected_network:%d",
             (unsigned)design_state,
             view_top_index,
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
             view_top_index,
             selected_network);

    switch (design_state) {
        case EditableElement::ElementState::des_Regular:
            if (view_top_index + Ladder::MaxViewPortCount < size()) {
                view_top_index++;
                Controller::UpdateUIViewTop(view_top_index);
            }
            break;

        case EditableElement::ElementState::des_Selected:
            (*this)[selected_network]->CancelSelection();

            if (!RemoveNetworkIfEmpty(selected_network)) {
                ScrollDown(&selected_network);
            }

            if (selected_network == (int)size()) {
                auto new_network = new Network(LogicItemState::lisActive);
                Append(new_network);
            }

            (*this)[selected_network]->Select();
            break;

        case EditableElement::ElementState::des_Editing:
            (*this)[selected_network]->SelectNext();
            break;

        case EditableElement::ElementState::des_AdvancedSelectMove:
            (*this)[selected_network]->SwitchToAdvancedSelectCopy();
            break;

        case EditableElement::ElementState::des_AdvancedSelectCopy:
            (*this)[selected_network]->SwitchToAdvancedSelectDelete();
            break;

        case EditableElement::ElementState::des_AdvancedSelectDelete:
            (*this)[selected_network]->SwitchToAdvancedSelectMove();
            break;

        case EditableElement::ElementState::des_AdvancedSelectDisable:
            (*this)[selected_network]->SwitchToAdvancedSelectMove();
            break;

        case EditableElement::ElementState::des_Moving:
            if (selected_network + 1 < (int)size()) {
                std::swap(at(selected_network), at(selected_network + 1));
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
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGI(TAG_Ladder,
             "HandleButtonPageDown, %u, view_top_index:%u, selected_network:%d",
             (unsigned)design_state,
             view_top_index,
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

    ESP_LOGI(TAG_Ladder,
             "HandleButtonSelect, %u, view_top_index:%u, selected_network:%d",
             (unsigned)design_state,
             (unsigned)view_top_index,
             selected_network);

    switch (design_state) {
        case EditableElement::ElementState::des_Regular: {
            if (size() == 0) {
                auto new_network = new Network(LogicItemState::lisActive);
                Append(new_network);
                break;
            }

            int32_t last_selected_network = Controller::GetLastUpdatedUISelected();
            last_selected_network =
                std::clamp(last_selected_network,
                           view_top_index,
                           (view_top_index + (int)Ladder::MaxViewPortCount) - 1);

            if (last_selected_network >= 0 && last_selected_network < (int)size()) {
                (*this)[last_selected_network]->Select();
                break;
            }

            (*this)[view_top_index]->Select();
            Controller::UpdateUISelected(view_top_index);

            break;
        }

        case EditableElement::ElementState::des_Selected:
            (*this)[selected_network]->BeginEditing();
            Controller::UpdateUISelected(selected_network);
            break;

        case EditableElement::ElementState::des_Editing:
            (*this)[selected_network]->Change();
            if (!(*this)[selected_network]->Editing()) {
                if (RemoveNetworkIfEmpty(selected_network)) {
                    selected_network = -1;
                }
                Store();
                Controller::UpdateUISelected(selected_network);
            }
            return;

        case EditableElement::ElementState::des_AdvancedSelectMove:
            (*this)[selected_network]->SwitchToMoving();
            break;

        case EditableElement::ElementState::des_AdvancedSelectCopy:
            (*this)[selected_network]->SwitchToCopying();
            break;

        case EditableElement::ElementState::des_AdvancedSelectDelete:
            (*this)[selected_network]->SwitchToDeleting();
            break;

        case EditableElement::ElementState::des_AdvancedSelectDisable:
            (*this)[selected_network]->SwitchToDisabling();
            break;

        case EditableElement::ElementState::des_Moving:
            (*this)[selected_network]->EndEditing();
            Store();
            break;

        case EditableElement::ElementState::des_Copying:
            (*this)[selected_network]->EndEditing();
            Duplicate(selected_network);
            Store();
            break;

        case EditableElement::ElementState::des_Deleting:
            (*this)[selected_network]->EndEditing();
            Delete(selected_network);
            Store();
            break;

        case EditableElement::ElementState::des_Disabling:
            (*this)[selected_network]->EndEditing();
            (*this)[selected_network]->SwitchState();
            Store();
            break;

        default:
            break;
    }
}

void Ladder::HandleButtonOption() {
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGI(TAG_Ladder,
             "HandleButtonOption, %u, selected_network:%d",
             (unsigned)design_state,
             selected_network);
    switch (design_state) {
        case EditableElement::ElementState::des_Editing:
            (*this)[selected_network]->Option();
            break;

        case EditableElement::ElementState::des_Selected:
            (*this)[selected_network]->SwitchToAdvancedSelectMove();
            break;

        case EditableElement::ElementState::des_AdvancedSelectMove:
            (*this)[selected_network]->EndEditing();
            break;

        case EditableElement::ElementState::des_AdvancedSelectCopy:
            (*this)[selected_network]->EndEditing();
            break;

        case EditableElement::ElementState::des_AdvancedSelectDelete:
            (*this)[selected_network]->EndEditing();
            break;

        case EditableElement::ElementState::des_AdvancedSelectDisable:
            (*this)[selected_network]->EndEditing();
            break;

        case EditableElement::ElementState::des_Moving:
            (*this)[selected_network]->EndEditing();
            break;

        case EditableElement::ElementState::des_Copying:
            (*this)[selected_network]->EndEditing();
            break;

        case EditableElement::ElementState::des_Deleting:
            (*this)[selected_network]->EndEditing();
            break;

        case EditableElement::ElementState::des_Disabling:
            (*this)[selected_network]->EndEditing();
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
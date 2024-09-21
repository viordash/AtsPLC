#include "LogicProgram/Ladder.h"
#include "Display/ScrollBar.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Ladder::Ladder() {
    view_top_index = 0;
    selected_network = -1;
    design_state = TEditableElementState::des_Regular;
}

Ladder::~Ladder() {
    RemoveAll();
}

void Ladder::RemoveAll() {
    while (!empty()) {
        auto it = begin();
        auto network = *it;
        erase(it);
        ESP_LOGD(TAG_Ladder, "delete network: %p", network);
        delete network;
    }
    view_top_index = 0;
    selected_network = -1;
}

bool Ladder::DoAction() {
    bool any_changes = false;
    for (auto it = begin(); it != end(); ++it) {
        any_changes |= (*it)->DoAction();
    }
    return any_changes;
}

IRAM_ATTR bool Ladder::Render(uint8_t *fb) {
    bool res = true;

    for (size_t i = 0; i < std::min(Ladder::MaxViewPortCount, size()); i++) {
        res &= at(i + view_top_index)->Render(fb, i);
    }

    ScrollBar::Render(fb, size(), Ladder::MaxViewPortCount, view_top_index);
    return res;
}

void Ladder::Append(Network *network) {
    ESP_LOGI(TAG_Ladder, "append network: %p", network);
    push_back(network);
}

bool Ladder::CanScrollAuto() {
    return view_top_index == size() - Ladder::MaxViewPortCount;
}

void Ladder::AutoScroll() {
    if (size() > Ladder::MaxViewPortCount) {
        view_top_index = size() - Ladder::MaxViewPortCount;
    }
}

void Ladder::ScrollUp() {
    ESP_LOGI(TAG_Ladder, "ScrollUp, %u", (unsigned)view_top_index);
    if (design_state == TEditableElementState::des_Selected && selected_network > view_top_index) {
        selected_network--;
    } else if (design_state != TEditableElementState::des_Editing && view_top_index > 0) {
        view_top_index--;
        selected_network--;
    }

    for (size_t i = 0; i < size(); i++) {
        auto network = (*this)[i];
        network->ChangeSelection(design_state == TEditableElementState::des_Selected
                                 && i == selected_network);
        network->ChangeEditing(design_state == TEditableElementState::des_Editing
                               && i == selected_network);
    }
}

void Ladder::ScrollDown() {
    ESP_LOGI(TAG_Ladder, "ScrollDown, %u", (unsigned)view_top_index);
    if (design_state == TEditableElementState::des_Selected && selected_network == view_top_index) {
        selected_network++;
    } else if (design_state != TEditableElementState::des_Editing
               && view_top_index + Ladder::MaxViewPortCount < size()) {
        view_top_index++;
        selected_network++;
    }

    for (size_t i = 0; i < size(); i++) {
        auto network = (*this)[i];
        network->ChangeSelection(design_state == TEditableElementState::des_Selected
                                 && i == selected_network);
        network->ChangeEditing(design_state == TEditableElementState::des_Editing
                               && i == selected_network);
    }
}

void Ladder::SwitchDesign() {
    ESP_LOGI(TAG_Ladder, "SwitchDesign, %u", (unsigned)design_state);
    switch (design_state) {
        case TEditableElementState::des_Regular:
            design_state = TEditableElementState::des_Selected;
            selected_network = view_top_index;
            break;
        case TEditableElementState::des_Selected:
            design_state = TEditableElementState::des_Regular;
            selected_network = -1;
            break;

        default:
            break;
    }

    for (size_t i = 0; i < size(); i++) {
        auto network = (*this)[i];
        network->ChangeSelection(design_state == TEditableElementState::des_Selected
                                 && i == selected_network);
        network->ChangeEditing(design_state == TEditableElementState::des_Editing
                               && i == selected_network);
    }
}

void Ladder::SwitchEditing() {
    ESP_LOGI(TAG_Ladder, "SwitchEditing, %u", (unsigned)design_state);
    switch (design_state) {
        case TEditableElementState::des_Selected:
            design_state = TEditableElementState::des_Editing;
            break;
        case TEditableElementState::des_Editing:
            design_state = TEditableElementState::des_Regular;
            break;

        default:
            break;
    }

    for (size_t i = 0; i < size(); i++) {
        auto network = (*this)[i];
        network->ChangeSelection(design_state == TEditableElementState::des_Selected
                                 && i == selected_network);
        network->ChangeEditing(design_state == TEditableElementState::des_Editing
                               && i == selected_network);
    }
}

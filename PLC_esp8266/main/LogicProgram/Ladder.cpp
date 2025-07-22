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
    for (size_t i = view_top_index; i < size(); i++) {
        uint8_t network_number = i - view_top_index;
        if (network_number >= Ladder::MaxViewPortCount) {
            break;
        }
        res &= at(i)->Render(fb, i - view_top_index);
    }

    ScrollBar::Render(fb, size(), Ladder::MaxViewPortCount, view_top_index);
    return res;
}

void Ladder::Append(Network *network) {
    ESP_LOGD(TAG_Ladder, "append network: %p", network);
    push_back(network);
}

void Ladder::Duplicate(int network_id) {
    ESP_LOGD(TAG_Ladder, "duplicate network id: %d", network_id);

    size_t size = (*this)[network_id]->Serialize(NULL, 0);
    if (size == 0) {
        ESP_LOGE(TAG_Ladder, "Duplicate error");
        return;
    }
    uint8_t *data = new uint8_t[size];

    if ((*this)[network_id]->Serialize(data, size) != size) {
        ESP_LOGE(TAG_Ladder, "Duplicate serialize error");
        delete[] data;
        return;
    }

    auto new_network = new Network(LogicItemState::lisActive);
    size_t network_readed = new_network->Deserialize(data, size);
    delete[] data;
    if (network_readed == 0) {
        ESP_LOGE(TAG_Ladder, "Duplicate deserialize error");
        delete new_network;
        return;
    }

    auto pos = begin();
    insert(std::next(pos, network_id), new_network);
}

void Ladder::Delete(int network_id) {
    auto pos = begin();
    auto it = std::next(pos, network_id);
    auto network = *it;
    erase(it);
    delete network;
}


void Ladder::SetViewTopIndex(int16_t index) {
    ESP_LOGI(TAG_Ladder, "SetViewTopIndex, index:%d", index);
    if (index < 0 || index + Ladder::MaxViewPortCount > size()) {
        return;
    }
    view_top_index = index;
}

void Ladder::SetSelectedNetworkIndex(int16_t index) {
    auto selected_network = GetSelectedNetwork();
    auto design_state = GetDesignState(selected_network);

    ESP_LOGI(TAG_Ladder,
             "SetSelectedNetworkIndex, %u, view_top_index:%u, selected_network:%d, index:%d",
             (unsigned)design_state,
             (unsigned)view_top_index,
             selected_network,
             index);
    if (index < 0 || index >= (int)size()) {
        return;
    }

    switch (design_state) {
        case EditableElement::ElementState::des_Regular:
            (*this)[index]->Select();
            break;

        case EditableElement::ElementState::des_Selected:
            (*this)[selected_network]->CancelSelection();

            (*this)[index]->Select();
            break;

        case EditableElement::ElementState::des_Editing:
            break;

        case EditableElement::ElementState::des_AdvancedSelectMove:
            break;

        case EditableElement::ElementState::des_AdvancedSelectCopy:
            break;

        case EditableElement::ElementState::des_AdvancedSelectDelete:
            break;

        case EditableElement::ElementState::des_Moving:
            break;

        case EditableElement::ElementState::des_Copying:
            break;

        case EditableElement::ElementState::des_Deleting:
            break;
    }
}

void Ladder::AtLeastOneNetwork() {
    if (!empty()) {
        return;
    }
    ESP_LOGI(TAG_Ladder, "requires at least one network");
    HandleButtonSelect();
}

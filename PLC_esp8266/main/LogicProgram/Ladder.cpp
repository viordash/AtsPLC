#include "LogicProgram/Ladder.h"
#include "Display/ScrollBar.h"
#include "LogicProgram/Controller.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lassert.h"
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Ladder::Ladder() {
    view_top_index = 0;
    frame_buffer_req_render = false;
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

IRAM_ATTR void Ladder::Render(FrameBuffer *fb) {
    for (size_t i = view_top_index; i < size(); i++) {
        uint8_t network_number = i - view_top_index;
        if (network_number >= Ladder::MaxViewPortCount) {
            break;
        }
        at(i)->Render(fb, i - view_top_index);
    }

    ScrollBar::Render(fb, size(), Ladder::MaxViewPortCount, view_top_index);

    fb->has_changes |= frame_buffer_req_render;
    frame_buffer_req_render = false;
}

void Ladder::Append(Network *network) {
    ESP_LOGD(TAG_Ladder, "append network: %p", network);
    push_back(network);
    frame_buffer_req_render = true;
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

    auto new_network = new Network();
    size_t network_readed = new_network->Deserialize(data, size);
    delete[] data;
    if (network_readed == 0) {
        ESP_LOGE(TAG_Ladder, "Duplicate deserialize error");
        delete new_network;
        return;
    }

    auto pos = begin();
    insert(std::next(pos, network_id), new_network);
    frame_buffer_req_render = true;
}

void Ladder::Delete(int network_id) {
    auto pos = begin();
    auto it = std::next(pos, network_id);
    auto network = *it;
    erase(it);
    delete network;
    frame_buffer_req_render = true;
}

void Ladder::SetViewTopIndex(int32_t index) {
    ESP_LOGI(TAG_Ladder, "SetViewTopIndex, index:%d", index);
    if (index < 0 || index + Ladder::MaxViewPortCount > size()) {
        return;
    }
    view_top_index = index;
    frame_buffer_req_render = true;
}

void Ladder::SetSelectedNetworkIndex(int32_t index) {
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

    index = std::clamp(index, view_top_index, (view_top_index + (int)Ladder::MaxViewPortCount) - 1);

    switch (design_state) {
        case EditableElement::ElementState::des_Regular:
            (*this)[index]->Select();
            Controller::DesignStart();
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

        case EditableElement::ElementState::des_AdvancedSelectDisable:
            break;

        case EditableElement::ElementState::des_Moving:
            break;

        case EditableElement::ElementState::des_Copying:
            break;

        case EditableElement::ElementState::des_Deleting:
            break;

        case EditableElement::ElementState::des_Disabling:
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

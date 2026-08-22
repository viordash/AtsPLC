#include "LogicProgram/Ladder.h"
#include "Display/ScrollBar.h"
#include "LogicProgram/Controller.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hotreload_service.h"
#include "lassert.h"
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Ladder::Ladder() {
    view_top_index = 0;
    frame_buffer_req_render = false;
    work_mode = WorkMode::Stop;
}

Ladder::~Ladder() {
    RemoveAll();
}

void Ladder::RemoveAll() {
    while (!items.empty()) {
        auto it = items.begin();
        auto network = *it;
        items.erase(it);
        ESP_LOGD(TAG_Ladder, "delete network: %p", network);
        delete network;
    }
    view_top_index = 0;
}

bool Ladder::DoAction() {
    bool any_changes = false;
    for (auto it = items.begin(); it != items.end(); ++it) {
        any_changes |= (*it)->DoAction();
    }
    return any_changes;
}

IRAM_ATTR void Ladder::Render(FrameBuffer *fb) {
    for (size_t i = view_top_index; i < items.size(); i++) {
        uint8_t network_number = i - view_top_index;
        if (network_number >= Ladder::MaxViewPortCount) {
            break;
        }
        items.at(i)->Render(fb, i - view_top_index);
    }

    ScrollBar::Render(fb,
                      SCROLLBAR_LEFT,
                      SCROLLBAR_TOP,
                      SCROLLBAR_HEIGHT,
                      items.size(),
                      Ladder::MaxViewPortCount,
                      view_top_index);

    fb->has_changes |= frame_buffer_req_render || Controller::InDesign();
    frame_buffer_req_render = false;
}

void Ladder::Append(Network *network) {
    ESP_LOGD(TAG_Ladder, "append network: %p", network);
    items.push_back(network);
    frame_buffer_req_render = true;
}

void Ladder::Duplicate(int network_id) {
    ESP_LOGD(TAG_Ladder, "duplicate network id: %d", network_id);

    size_t buf_size = items[network_id]->Serialize(NULL, 0);
    if (buf_size == 0) {
        ESP_LOGE(TAG_Ladder, "Duplicate error");
        return;
    }
    uint8_t *data = new uint8_t[buf_size];

    if (items[network_id]->Serialize(data, buf_size) != buf_size) {
        ESP_LOGE(TAG_Ladder, "Duplicate serialize error");
        delete[] data;
        return;
    }

    auto new_network = work_mode == WorkMode::Stop ? new Network(NetworkState::nsStopFromPassive)
                                                   : new Network(NetworkState::nsPassive);
    size_t network_readed = new_network->Deserialize(data, buf_size);
    delete[] data;
    if (network_readed == 0) {
        ESP_LOGE(TAG_Ladder, "Duplicate deserialize error");
        delete new_network;
        return;
    }

    auto pos = items.begin();
    items.insert(std::next(pos, network_id), new_network);
    frame_buffer_req_render = true;
}

void Ladder::Delete(int network_id) {
    auto pos = items.begin();
    auto it = std::next(pos, network_id);
    auto network = *it;
    items.erase(it);
    delete network;
    frame_buffer_req_render = true;
}

void Ladder::SetViewTopIndex(int32_t index) {
    ESP_LOGI(TAG_Ladder, "SetViewTopIndex, index:%d", index);
    if (index < 0 || index + Ladder::MaxViewPortCount > items.size()) {
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
    if (index < 0 || index >= (int)items.size()) {
        return;
    }

    index = std::clamp(index, view_top_index, (view_top_index + (int)Ladder::MaxViewPortCount) - 1);

    switch (design_state) {
        case EditableElement::ElementState::des_Regular:
            Controller::DesignStart(work_mode);
            if (!Controller::InDesign()) {
                ESP_LOGE(TAG_Ladder, "Designing in Run mode is prohibited");
                break;
            }
            items[index]->Select();
            break;

        case EditableElement::ElementState::des_Selected:
            items[selected_network]->CancelSelection();

            items[index]->Select();
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
    if (!items.empty()) {
        return;
    }
    ESP_LOGI(TAG_Ladder, "requires at least one network");
    HandleButtonSelect();
}

WorkMode Ladder::GetWorkMode() {
    return work_mode;
}

void Ladder::ChangeWorkMode(WorkMode new_mode, bool enable_debug) {
    NetworkState active_state;
    NetworkState passive_state;
    if (new_mode == WorkMode::Run) {
        active_state = NetworkState::nsActive;
        passive_state = NetworkState::nsPassive;
        work_mode = WorkMode::Run;
    } else {
        active_state = NetworkState::nsStopFromActive;
        passive_state = NetworkState::nsStopFromPassive;
        work_mode = WorkMode::Stop;
    }

    SAFETY_HOTRELOAD({
        hotreload->enable_debug = enable_debug;
        store_hotreload();
    });

    for (auto &network : items) {
        switch (network->GetState()) {
            case NetworkState::nsActive:
            case NetworkState::nsStopFromActive:
                network->ChangeState(active_state);
                break;

            case NetworkState::nsPassive:
            case NetworkState::nsStopFromPassive:
                network->ChangeState(passive_state);
                break;
        }
    }

    Store();
    ESP_LOGI(TAG_Ladder,
             "ChangeWorkMode work_mode:%u, enable_debug:%u",
             (unsigned int)work_mode,
             (unsigned int)enable_debug);
}

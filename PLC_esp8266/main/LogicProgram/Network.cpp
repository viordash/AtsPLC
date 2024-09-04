#include "LogicProgram/Network.h"
#include "Display/display.h"
#include "LogicProgram/Serializer/LogicElementFactory.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_Network = "Network";

Network::Network() : LogicElement() {
    this->network_number = 0;
}

Network::~Network() {
    while (!empty()) {
        auto it = begin();
        auto element = *it;
        erase(it);
        ESP_LOGD(TAG_Network, "delete elem: %p", element);
        delete element;
    }
}

void Network::SetNumber(uint8_t network_number) {
    this->network_number = network_number;
}

void Network::ChangeState(LogicItemState state) {
    this->state = state;
}

bool Network::DoAction() {
    return DoAction(false, state);
}

bool Network::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    bool any_changes = false;

    for (auto it = begin(); it != end(); ++it) {
        auto element = *it;
        prev_elem_changed = element->DoAction(prev_elem_changed, prev_elem_state);
        prev_elem_state = element->state;
        any_changes |= prev_elem_changed;
    }
    return any_changes;
}

bool Network::Render(uint8_t *fb) {
    Point start_point = { 0,
                          (uint8_t)(INCOME_RAIL_TOP + INCOME_RAIL_HEIGHT * network_number
                                    + INCOME_RAIL_OUTCOME_TOP) };
    return Render(fb, state, &start_point);
}

bool Network::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    bool res = true;
    switch (prev_elem_state) {
        case LogicItemState::lisActive:
            res = draw_active_income_rail(fb, network_number);
            break;

        default:
            res = draw_passive_income_rail(fb, network_number);
            break;
    }
    start_point->x += INCOME_RAIL_WIDTH;
    for (auto it = begin(); res && it != end(); ++it) {
        auto element = *it;
        res = element->Render(fb, prev_elem_state, start_point);
        prev_elem_state = element->state;
    }

    if (!res) {
        return res;
    }
    res = draw_outcome_rail(fb, network_number);

    return res;
}

void Network::Append(LogicElement *element) {
    ESP_LOGI(TAG_Network, "append elem: %p", element);
    push_back(element);
}

size_t Network::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = GetElementType();

    uint16_t elements_count = size();
    if (elements_count < Network::MinElementsCount) {
        return 0;
    }
    if (elements_count > Network::MaxElementsCount) {
        return 0;
    }

    if (!WriteRecord(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }

    if (!WriteRecord(&state, sizeof(state), buffer, buffer_size, &writed)) {
        return 0;
    }

    if (!WriteRecord(&elements_count, sizeof(elements_count), buffer, buffer_size, &writed)) {
        return 0;
    }

    for (auto it = begin(); it != end(); ++it) {
        auto *element = *it;
        uint8_t *p;
        bool just_obtain_size = buffer == NULL;
        if (!just_obtain_size) {
            p = &buffer[writed];
        } else {
            p = NULL;
        }

        size_t element_writed = element->Serialize(p, buffer_size - writed);
        bool element_serialize_error = element_writed == 0;
        if (element_serialize_error) {
            return 0;
        }
        writed += element_writed;
    }

    return writed;
}

size_t Network::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;

    LogicItemState _state;
    if (!ReadRecord(&_state, sizeof(_state), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (!ValidateLogicItemState(_state)) {
        return 0;
    }

    uint16_t elements_count;
    if (!ReadRecord(&elements_count, sizeof(elements_count), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (elements_count < Network::MinElementsCount) {
        return 0;
    }
    if (elements_count > Network::MaxElementsCount) {
        return 0;
    }

    state = _state;
    reserve(elements_count);
    for (size_t i = 0; i < elements_count; i++) {
        TvElement tvElement;
        if (!ReadRecord(&tvElement, sizeof(tvElement), buffer, buffer_size, &readed)) {
            return 0;
        }

        auto element = LogicElementFactory::Create(tvElement.type);
        if (element == NULL) {
            return 0;
        }

        size_t element_readed = element->Deserialize(&buffer[readed], buffer_size - readed);
        if (element_readed == 0) {
            delete element;
            return 0;
        }
        readed += element_readed;
        Append(element);
    }
    return readed;
}

TvElementType Network::GetElementType() {
    return TvElementType::et_Network;
}
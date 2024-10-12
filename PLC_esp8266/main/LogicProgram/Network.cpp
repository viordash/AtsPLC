#include "LogicProgram/Network.h"
#include "Display/display.h"
#include "LogicProgram/ElementsBox.h"
#include "LogicProgram/Serializer/LogicElementFactory.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_Network = "Network";

Network::Network(LogicItemState state) : EditableElement() {
    ChangeState(state);
}
Network::Network() : Network(LogicItemState::lisPassive) {
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

void Network::ChangeState(LogicItemState state) {
    this->state = state;
}

bool Network::DoAction() {
    bool any_changes = false;
    bool prev_elem_changed = true;
    LogicItemState prev_elem_state = state;

    for (auto it = begin(); it != end(); ++it) {
        auto element = *it;
        prev_elem_changed = element->DoAction(prev_elem_changed, prev_elem_state);
        prev_elem_state = element->state;
        any_changes |= prev_elem_changed;
    }
    return any_changes;
}

IRAM_ATTR bool Network::Render(uint8_t *fb, uint8_t network_number) {
    Point start_point = { 0,
                          (uint8_t)(INCOME_RAIL_TOP + INCOME_RAIL_HEIGHT * network_number
                                    + INCOME_RAIL_NETWORK_TOP) };
    bool res = true;

    ESP_LOGD(TAG_Network, "Render: %u, x:%u, y:%u", network_number, start_point.x, start_point.y);

    switch (state) {
        case LogicItemState::lisActive:
            res = draw_active_income_rail(fb, start_point.x, start_point.y);
            break;

        default:
            res = draw_passive_income_rail(fb, start_point.x, start_point.y);
            break;
    }

    Point editable_sign_start_point = start_point;
    bool any_child_is_edited = false;
    LogicItemState prev_elem_state = state;
    start_point.x += INCOME_RAIL_WIDTH;
    for (auto it = begin(); res && it != end(); ++it) {
        auto element = *it;
        if (element->Selected() || element->Editing()) {
            any_child_is_edited = true;
        }
        res = element->Render(fb, prev_elem_state, &start_point);
        prev_elem_state = element->state;
    }

    if (res && !any_child_is_edited) {
        res = EditableElement::Render(fb, &editable_sign_start_point);
    }

    if (res) {
        res = draw_outcome_rail(fb, start_point.x, start_point.y);
    }

    return res;
}

void Network::Append(LogicElement *element) {
    ESP_LOGI(TAG_Network, "append elem: %p", element);
    push_back(element);
}

size_t Network::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;

    uint16_t elements_count = size();
    if (elements_count < Network::MinElementsCount) {
        return 0;
    }
    if (elements_count > Network::MaxElementsCount) {
        return 0;
    }

    if (!Record::Write(&state, sizeof(state), buffer, buffer_size, &writed)) {
        return 0;
    }

    if (!Record::Write(&elements_count, sizeof(elements_count), buffer, buffer_size, &writed)) {
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
    if (!Record::Read(&_state, sizeof(_state), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (!ValidateLogicItemState(_state)) {
        return 0;
    }

    uint16_t elements_count;
    if (!Record::Read(&elements_count, sizeof(elements_count), buffer, buffer_size, &readed)) {
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
        if (!Record::Read(&tvElement, sizeof(tvElement), buffer, buffer_size, &readed)) {
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

void Network::HandleButtonUp() {
    auto selected_element = GetSelectedElement();

    if (selected_element >= 0) {
        if ((*this)[selected_element]->Editing()) {
            static_cast<ElementsBox *>((*this)[selected_element])->HandleButtonUp();
            return;
        }
    }

    ESP_LOGI(TAG_Network,
             "HandleButtonUp, %u, selected_element:%d",
             (unsigned)editable_state,
             selected_element);

    bool edit_this_network = selected_element < 0;
    if (edit_this_network) {
        switch (state) {
            case LogicItemState::lisPassive:
                ChangeState(LogicItemState::lisActive);
                break;
            default:
                ChangeState(LogicItemState::lisPassive);
                break;
        }
        return;
    }
}

void Network::HandleButtonDown() {
    auto selected_element = GetSelectedElement();

    if (selected_element >= 0) {
        if ((*this)[selected_element]->Editing()) {
            static_cast<ElementsBox *>((*this)[selected_element])->HandleButtonDown();
            return;
        }
    }
    ESP_LOGI(TAG_Network,
             "HandleButtonDown, %u, selected_element:%d",
             (unsigned)editable_state,
             selected_element);

    bool edit_this_network = selected_element < 0;
    if (edit_this_network) {
        switch (state) {
            case LogicItemState::lisPassive:
                ChangeState(LogicItemState::lisActive);
                break;
            default:
                ChangeState(LogicItemState::lisPassive);
                break;
        }
        return;
    }
}

void Network::HandleButtonSelect() {
    auto selected_element = GetSelectedElement();

    if (selected_element >= 0) {
        if ((*this)[selected_element]->Editing()) {
            static_cast<ElementsBox *>((*this)[selected_element])->HandleButtonSelect();
            return;
        }
        (*this)[selected_element]->CancelSelection();
    }
    ESP_LOGI(TAG_Network,
             "HandleButtonSelect, %u, selected_element:%d, size:%u",
             (unsigned)editable_state,
             selected_element,
             (unsigned)size());

    selected_element++;

    if (selected_element >= (int)size()) {
        selected_element = -1;
    }
    if (selected_element >= 0) {
        (*this)[selected_element]->Select();
    }
}

void Network::HandleButtonOption() {
    auto selected_element = GetSelectedElement();
    ESP_LOGI(TAG_Network,
             "HandleButtonOption, %u, selected_element:%d",
             (unsigned)editable_state,
             selected_element);

    bool edit_this_network = selected_element < 0;
    if (edit_this_network) {
        EndEditing();
        return;
    }

    if ((*this)[selected_element]->Selected()) {
        auto stored_element = (*this)[selected_element];
        (*this)[selected_element] = new ElementsBox(100, stored_element);
        (*this)[selected_element]->BeginEditing();

    } else if ((*this)[selected_element]->Editing()) {
        (*this)[selected_element]->EndEditing();
        auto editedElement =
            static_cast<ElementsBox *>((*this)[selected_element])->GetSelectedElement();

        delete (*this)[selected_element];
        (*this)[selected_element] = editedElement;
        (*this)[selected_element]->Select();
    }
}

void Network::EndEditing() {
    auto selected_element = GetSelectedElement();
    if (selected_element >= 0) {
        (*this)[selected_element]->CancelSelection();
    }
    EditableElement::EndEditing();
}

int Network::GetSelectedElement() {
    for (int i = 0; i < (int)size(); i++) {
        auto element = (*this)[i];
        if (element->Selected() || element->Editing()) {
            return i;
        }
    }
    return -1;
}

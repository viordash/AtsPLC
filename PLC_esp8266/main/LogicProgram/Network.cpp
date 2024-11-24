#include "LogicProgram/Network.h"
#include "Display/display.h"
#include "LogicProgram/ElementsBox.h"
#include "LogicProgram/Inputs/CommonComparator.h"
#include "LogicProgram/Inputs/CommonInput.h"
#include "LogicProgram/Inputs/CommonTimer.h"
#include "LogicProgram/Outputs/CommonOutput.h"
#include "LogicProgram/Serializer/LogicElementFactory.h"
#include "LogicProgram/Serializer/Record.h"
#include "LogicProgram/Wire.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_Network = "Network";

Network::Network(LogicItemState state) : EditableElement() {
    fill_wire = 0;
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
    state_changed = true;
}

bool Network::DoAction() {
    bool any_changes = false;
    bool prev_elem_changed = state_changed;
    state_changed = false;
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

    auto it = begin();
    while (res && it != end()) {
        auto element = *it;
        if (CommonInput::TryToCast(element) == NULL && CommonTimer::TryToCast(element) == NULL
            && Wire::TryToCast(element) == NULL) {
            break;
        }
        it++;
        if (element->Selected() || element->Editing()) {
            any_child_is_edited = true;
        }
        res = element->Render(fb, prev_elem_state, &start_point);
        prev_elem_state = element->state;
    }

    Point end_point = { OUTCOME_RAIL_RIGHT, start_point.y };
    while (res && it != end()) {
        auto element = *it;
        if (CommonOutput::TryToCast(element) == NULL) {
            break;
        }
        it++;
        if (element->Selected() || element->Editing()) {
            any_child_is_edited = true;
        }
        res = element->Render(fb, prev_elem_state, &end_point);
        prev_elem_state = element->state;
    }

    if (res && !any_child_is_edited) {
        res = EditableElement::Render(fb, &editable_sign_start_point);
    }

    fill_wire = end_point.x - start_point.x;

    if (res) {
        if (prev_elem_state == LogicItemState::lisActive) {
            res = draw_active_network(fb, start_point.x, start_point.y, fill_wire);
        } else {
            res = draw_passive_network(fb, start_point.x, start_point.y, fill_wire, false);
        }
    }

    if (res) {
        res = draw_outcome_rail(fb, OUTCOME_RAIL_RIGHT, start_point.y);
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

void Network::SelectPrior() {
    auto selected_element = GetSelectedElement();

    if (selected_element >= 0) {
        if ((*this)[selected_element]->Editing()) {
            static_cast<ElementsBox *>((*this)[selected_element])->SelectPrior();
            return;
        }
        (*this)[selected_element]->CancelSelection();
    }
    selected_element--;
    if (selected_element < -1) {
        selected_element = size() - 1;
    }
    if (selected_element >= 0) {
        (*this)[selected_element]->Select();
    }

    ESP_LOGI(TAG_Network,
             "SelectPrior, %u, selected_element:%d",
             (unsigned)editable_state,
             selected_element);
}

void Network::SelectNext() {
    auto selected_element = GetSelectedElement();

    if (selected_element >= 0) {
        if ((*this)[selected_element]->Editing()) {
            static_cast<ElementsBox *>((*this)[selected_element])->SelectNext();
            return;
        }
        (*this)[selected_element]->CancelSelection();
    }
    selected_element++;
    if (selected_element >= (int)size()) {
        selected_element = -1;
    } else {
        (*this)[selected_element]->Select();
    }

    ESP_LOGI(TAG_Network,
             "SelectNext, %u, selected_element:%d",
             (unsigned)editable_state,
             selected_element);
}

void Network::PageUp() {
    auto selected_element = GetSelectedElement();

    if (selected_element >= 0) {
        if ((*this)[selected_element]->Editing()) {
            static_cast<ElementsBox *>((*this)[selected_element])->PageUp();
            return;
        }
    }
}

void Network::PageDown() {
    auto selected_element = GetSelectedElement();

    if (selected_element >= 0) {
        if ((*this)[selected_element]->Editing()) {
            static_cast<ElementsBox *>((*this)[selected_element])->PageDown();
            return;
        }
    }
}

void Network::Change() {
    auto selected_element = GetSelectedElement();
    ESP_LOGI(TAG_Network,
             "Change, %u, selected_element:%d",
             (unsigned)editable_state,
             selected_element);

    bool edit_this_network = selected_element < 0;
    if (edit_this_network) {
        EndEditing();
        return;
    }

    if ((*this)[selected_element]->Selected()) {
        auto source_element = (*this)[selected_element];
        bool hide_output_elements = HasOutputElement() && CommonOutput::TryToCast(source_element) == NULL;
        ESP_LOGI(TAG_Network, "hide_output_elements:%u", hide_output_elements);
        auto elementBox = new ElementsBox(fill_wire, source_element, hide_output_elements);
        elementBox->BeginEditing();
        (*this)[selected_element] = elementBox;
        delete source_element;

    } else if ((*this)[selected_element]->Editing()) {
        auto elementBox = static_cast<ElementsBox *>((*this)[selected_element]);

        elementBox->Change();
        if (elementBox->EditingCompleted()) {
            elementBox->EndEditing();
            auto editedElement = elementBox->GetSelectedElement();
            delete elementBox;
            (*this)[selected_element] = editedElement;

            RemoveSpaceForNewElement();
            AddSpaceForNewElement();
        }
    }
}

bool Network::EnoughSpaceForNewElement(LogicElement *new_element) {
    bool hide_output_elements = HasOutputElement();
    ElementsBox elementBox(fill_wire, new_element, hide_output_elements);
    new_element->EndEditing();
    bool not_enough =
        elementBox.size() == 0
        || (elementBox.size() == 1
            && elementBox.GetSelectedElement()->GetElementType() == TvElementType::et_Wire);
    delete elementBox.GetSelectedElement();
    return !not_enough;
}

void Network::AddSpaceForNewElement() {
    auto wire = new Wire();
    uint8_t wire_width = fill_wire / 2 + 1;
    if (wire_width > WIRE_STANDART_WIDTH) {
        wire_width = WIRE_STANDART_WIDTH;
    }
    wire->SetWidth(wire_width);
    if (EnoughSpaceForNewElement(wire)) {
        ESP_LOGI(TAG_Network, "insert wire element");

        auto it = begin();
        while (it != end()) {
            auto element = *it;
            bool is_output_element =
                CommonInput::TryToCast(element) == NULL && CommonTimer::TryToCast(element) == NULL;
            if (is_output_element) {
                break;
            }
            it++;
        }
        insert(it, wire);

    } else {
        delete wire;
    }
}

void Network::RemoveSpaceForNewElement() {
    auto it = begin();
    while (it != end()) {
        auto element = *it;
        auto as_wire = Wire::TryToCast(element);
        if (as_wire != NULL) {
            fill_wire += as_wire->GetWidth();
            it = erase(it);
            delete as_wire;
            ESP_LOGI(TAG_Network, "remove wire element");
        } else {
            it++;
        }
    }
}

bool Network::HasOutputElement() {
    for (auto it = begin(); it != end(); ++it) {
        auto element = *it;
        bool is_output_element = CommonOutput::TryToCast(element) != NULL;
        if (is_output_element) {
            return true;
        }
    }
    return false;
}

void Network::BeginEditing() {
    ESP_LOGI(TAG_Network, "BeginEditing");

    AddSpaceForNewElement();
    EditableElement::BeginEditing();
}

void Network::EndEditing() {
    auto selected_element = GetSelectedElement();
    if (selected_element >= 0) {
        (*this)[selected_element]->CancelSelection();
    }

    ESP_LOGI(TAG_Network, "EndEditing");
    EditableElement::EndEditing();

    RemoveSpaceForNewElement();
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

void Network::SwitchState() {
    switch (state) {
        case LogicItemState::lisPassive:
            ChangeState(LogicItemState::lisActive);
            break;
        default:
            ChangeState(LogicItemState::lisPassive);
            break;
    }
}
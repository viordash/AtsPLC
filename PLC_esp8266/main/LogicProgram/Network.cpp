#include "LogicProgram/Network.h"
#include "Display/display.h"
#include "LogicProgram/ElementsBox.h"
#include "LogicProgram/LogicProgram.h"
#include "LogicProgram/Serializer/LogicElementFactory.h"
#include "LogicProgram/Serializer/Record.h"
#include "LogicProgram/Wire.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lassert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_Network = "Network";

Network::Network(NetworkState state) : EditableElement() {
    fill_wire = 0;
    ChangeState(state);
    frame_buffer_req_render.store(false, std::memory_order_relaxed);
}
Network::Network() : Network(NetworkState::nsPassive) {
}

Network::~Network() {
    RemoveAll();
}

void Network::RemoveAll() {
    while (!items.empty()) {
        auto it = items.begin();
        auto element = *it;
        items.erase(it);
        ESP_LOGD(TAG_Network, "delete elem: %p", element);
        delete element;
    }
}

bool Network::Empty() const {
    return items.empty();
}

size_t Network::Size() const {
    return items.size();
}

LogicElement *&Network::At(size_t index) {
    return items.at(index);
}

LogicElement *&Network::operator[](size_t index) {
    return items[index];
}

LogicElement *const &Network::operator[](size_t index) const {
    return items[index];
}

void Network::ChangeState(NetworkState state) {
    this->state = state;
    state_changed = true;
}

NetworkState Network::GetState() {
    return state;
}

bool Network::DoAction() {
    bool any_changes = false;
    bool prev_elem_changed = state_changed;
    state_changed = false;
    LogicItemState prev_elem_state = Convert2LogicItemState(state);

    for (auto it = items.begin(); it != items.end(); ++it) {
        auto element = *it;
        prev_elem_changed = element->DoAction(prev_elem_changed, prev_elem_state);
        prev_elem_state = element->state;
        any_changes |= prev_elem_changed;
    }

    frame_buffer_req_render.store(frame_buffer_req_render.load(std::memory_order_relaxed)
                                      | any_changes,
                                  std::memory_order_relaxed);
    return any_changes;
}

IRAM_ATTR void Network::Render(FrameBuffer *fb, uint8_t network_number) {
    Point start_point = { 0,
                          (uint8_t)(INCOME_RAIL_TOP + INCOME_RAIL_HEIGHT * network_number
                                    + INCOME_RAIL_NETWORK_TOP) };
    ESP_LOGD(TAG_Network, "Render: %u, x:%u, y:%u", network_number, start_point.x, start_point.y);

    switch (state) {
        case NetworkState::nsActive:
        case NetworkState::nsStopFromActive:
            ASSERT(draw_active_income_rail(fb, start_point.x, start_point.y));
            break;

        case NetworkState::nsPassive:
        case NetworkState::nsStopFromPassive:
            ASSERT(draw_passive_income_rail(fb, start_point.x, start_point.y));
            break;
    }

    Point editable_sign_start_point = start_point;
    bool any_child_is_edited = false;
    LogicItemState prev_elem_state = Convert2LogicItemState(state);
    start_point.x += INCOME_RAIL_WIDTH;

    auto it = items.begin();
    while (it != items.end()) {
        auto element = *it;
        if (IsOutputElement(element->GetElementType())) {
            break;
        }

        auto *continuationIn = ContinuationIn::TryToCast(element);
        if (continuationIn != NULL) {
            break;
        }

        it++;
        if (element->Selected() || element->Editing()) {
            any_child_is_edited = true;
        }
        element->Render(fb, prev_elem_state, &start_point);
        prev_elem_state = element->state;
    }

    Point end_point = { OUTCOME_RAIL_RIGHT, start_point.y };
    while (it != items.end()) {
        auto element = *it;
        if (!IsOutputElement(element->GetElementType())) {
            auto *continuationIn = ContinuationIn::TryToCast(element);
            if (continuationIn == NULL) {
                break;
            }
        }

        it++;
        if (element->Selected() || element->Editing()) {
            any_child_is_edited = true;
        }
        element->Render(fb, prev_elem_state, &end_point);
        prev_elem_state = element->state;
    }

    if (!any_child_is_edited) {
        EditableElement::Render(fb, &editable_sign_start_point);
    }

    fill_wire = end_point.x - start_point.x;

    switch (prev_elem_state) {
        case LogicItemState::lisActive:
        case LogicItemState::lisStop:
            ASSERT(draw_active_network(fb, start_point.x, start_point.y, fill_wire));
            break;
        case LogicItemState::lisPassive:
            ASSERT(draw_passive_network(fb, start_point.x, start_point.y, fill_wire, false));
            break;
    }

    switch (state) {
        case NetworkState::nsActive:
        case NetworkState::nsPassive:
            ASSERT(draw_outcome_rail(fb, OUTCOME_RAIL_RIGHT, start_point.y));
            break;

        case NetworkState::nsStopFromActive:
        case NetworkState::nsStopFromPassive:
            ASSERT(draw_passive_outcome_rail(fb, OUTCOME_RAIL_RIGHT, start_point.y));
            break;
    }

    fb->has_changes |= frame_buffer_req_render.load(std::memory_order_relaxed);
    frame_buffer_req_render.store(false, std::memory_order_relaxed);
}

void Network::Append(LogicElement *element) {
    ESP_LOGD(TAG_Network, "append elem: %p", element);
    items.push_back(element);
}

size_t Network::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;

    uint16_t elements_count = items.size();
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

    for (auto it = items.begin(); it != items.end(); ++it) {
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

    NetworkState _state;
    if (!Record::Read(&_state, sizeof(_state), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (!ValidateNetworkState(_state)) {
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
    items.reserve(elements_count);
    for (size_t i = 0; i < elements_count; i++) {
        TvElement tvElement;
        if (!Record::Read(&tvElement, sizeof(tvElement), buffer, buffer_size, &readed)) {
            RemoveAll();
            return 0;
        }

        auto element = LogicElementFactory::Create(tvElement.type);
        if (element == NULL) {
            RemoveAll();
            return 0;
        }

        size_t element_readed = element->Deserialize(&buffer[readed], buffer_size - readed);
        if (element_readed == 0) {
            delete element;
            RemoveAll();
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
        if (items[selected_element]->Editing()) {
            static_cast<ElementsBox *>(items[selected_element])->SelectPrior();
            return;
        }
        items[selected_element]->CancelSelection();
    }
    selected_element--;
    if (selected_element < -1) {
        selected_element = items.size() - 1;
    }
    if (selected_element >= 0) {
        items[selected_element]->Select();
    }

    ESP_LOGI(TAG_Network,
             "SelectPrior, %u, selected_element:%d",
             (unsigned)editable_state,
             selected_element);
}

void Network::SelectNext() {
    auto selected_element = GetSelectedElement();

    if (selected_element >= 0) {
        if (items[selected_element]->Editing()) {
            static_cast<ElementsBox *>(items[selected_element])->SelectNext();
            return;
        }
        items[selected_element]->CancelSelection();
    }
    selected_element++;
    if (selected_element >= (int)items.size()) {
        selected_element = -1;
    } else {
        items[selected_element]->Select();
    }

    ESP_LOGI(TAG_Network,
             "SelectNext, %u, selected_element:%d",
             (unsigned)editable_state,
             selected_element);
}

void Network::PageUp() {
    auto selected_element = GetSelectedElement();

    if (selected_element >= 0) {
        if (items[selected_element]->Editing()) {
            static_cast<ElementsBox *>(items[selected_element])->PageUp();
            return;
        }
    }
}

void Network::PageDown() {
    auto selected_element = GetSelectedElement();

    if (selected_element >= 0) {
        if (items[selected_element]->Editing()) {
            static_cast<ElementsBox *>(items[selected_element])->PageDown();
            return;
        }
    }
}

bool Network::OptionShowOutputElement(LogicElement *selected_element) {
    bool last_is_ContinuationIn =
        selected_element != items.back() && ContinuationIn::TryToCast(items.back()) != NULL;
    if (last_is_ContinuationIn) {
        return false;
    }

    if (!HasOutputElement()) {
        return true;
    }

    if (IsOutputElement(selected_element->GetElementType())) {
        return true;
    }
    return false;
}

bool Network::OptionShowContinuationIn(LogicElement *selected_element) {
    if (selected_element == items.back()) {
        return true;
    }

    bool last_is_wire = Wire::TryToCast(items.back()) != NULL;
    if (!last_is_wire) {
        return false;
    }

    bool is_before_last = items.size() >= 2 && (selected_element == *(items.rbegin() + 1));
    if (is_before_last) {
        return true;
    }
    return false;
}

bool Network::OptionShowContinuationOut(LogicElement *selected_element) {
    return selected_element == items.front();
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

    if (items[selected_element]->Selected()) {
        auto source_element = items[selected_element];
        ElementsBox::Options options{};
        if (OptionShowOutputElement(items[selected_element])) {
            options = (ElementsBox::Options)(options | ElementsBox::Options::show_output_elements);
        }
        if (OptionShowContinuationIn(items[selected_element])) {
            options = (ElementsBox::Options)(options | ElementsBox::Options::show_continuation_in);
        }
        if (OptionShowContinuationOut(items[selected_element])) {
            options = (ElementsBox::Options)(options | ElementsBox::Options::show_continuation_out);
        }

        ESP_LOGI(TAG_Network, "ElementsBox::Options:0x%08X", options);
        auto elementBox = new ElementsBox(fill_wire, source_element, options);
        elementBox->BeginEditing();
        items[selected_element] = elementBox;

    } else if (items[selected_element]->Editing()) {
        auto elementBox = static_cast<ElementsBox *>(items[selected_element]);

        elementBox->Change();
        if (elementBox->EditingCompleted()) {
            elementBox->EndEditing();
            auto editedElement = elementBox->GetSelectedElement();
            delete elementBox;
            items[selected_element] = editedElement;

            RemoveSpaceForNewElement();
            AddSpaceForNewElement();
        }
    }
}

bool Network::EnoughSpaceForNewElement(LogicElement *new_element) {
    ElementsBox::Options options{};
    if (!HasOutputElement()) {
        options = (ElementsBox::Options)(options | ElementsBox::Options::show_output_elements);
    }
    ElementsBox elementBox(fill_wire, new_element, options);
    new_element->EndEditing();
    bool not_enough =
        elementBox.size() == 0
        || (elementBox.size() == 1
            && elementBox.GetSelectedElement()->GetElementType() == TvElementType::et_Wire);
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
        LogicItemState wire_state = Convert2LogicItemState(state);
        auto it = items.begin();
        while (it != items.end()) {
            auto element = *it;
            if (IsOutputElement(element->GetElementType())) {
                break;
            }
            auto *continuationIn = ContinuationIn::TryToCast(element);
            if (continuationIn != NULL) {
                break;
            }
            wire_state = element->state;
            it++;
        }
        wire->state = wire_state;
        items.insert(it, wire);

    } else {
        delete wire;
    }
}

void Network::RemoveSpaceForNewElement() {
    auto it = items.begin();
    while (it != items.end()) {
        auto element = *it;
        auto as_wire = Wire::TryToCast(element);
        if (as_wire != NULL) {
            fill_wire += as_wire->GetWidth();
            it = items.erase(it);
            delete as_wire;
            ESP_LOGI(TAG_Network, "remove wire element");
        } else {
            it++;
        }
    }
}

bool Network::HasOutputElement() {
    for (auto it = items.begin(); it != items.end(); ++it) {
        auto element = *it;
        if (IsOutputElement(element->GetElementType())) {
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
        items[selected_element]->CancelSelection();
    }

    ESP_LOGI(TAG_Network, "EndEditing");
    EditableElement::EndEditing();

    RemoveSpaceForNewElement();
}

void Network::Option() {
    auto selected_element = GetSelectedElement();

    ESP_LOGI(TAG_Network, "Option, selected_element:%d", selected_element);
    if (selected_element >= 0) {
        if (items[selected_element]->Editing()) {
            auto elementBox = static_cast<ElementsBox *>(items[selected_element]);
            elementBox->Option();
            if (elementBox->EditingCompleted()) {
                elementBox->EndEditing();
                auto editedElement = elementBox->GetSelectedElement();
                delete elementBox;
                items[selected_element] = editedElement;

                RemoveSpaceForNewElement();
                AddSpaceForNewElement();
            }
            return;
        }
    }
}

int Network::GetSelectedElement() {
    for (int i = 0; i < (int)items.size(); i++) {
        auto element = items[i];
        if (element->Selected() || element->Editing()) {
            return i;
        }
    }
    return -1;
}

void Network::SwitchState() {
    switch (state) {
        case NetworkState::nsPassive:
            ChangeState(NetworkState::nsActive);
            break;

        case NetworkState::nsActive:
            ChangeState(NetworkState::nsPassive);
            break;

        case NetworkState::nsStopFromPassive:
            ChangeState(NetworkState::nsStopFromActive);
            break;

        case NetworkState::nsStopFromActive:
            ChangeState(NetworkState::nsStopFromPassive);
            break;
    }
}

void Network::SwitchToAdvancedSelectMove() {
    ESP_LOGI(TAG_Network, "SwitchToAdvancedSelectMove");
    editable_state = EditableElement::ElementState::des_AdvancedSelectMove;
}

void Network::SwitchToAdvancedSelectCopy() {
    ESP_LOGI(TAG_Network, "SwitchToAdvancedSelectCopy");
    editable_state = EditableElement::ElementState::des_AdvancedSelectCopy;
}

void Network::SwitchToAdvancedSelectDelete() {
    ESP_LOGI(TAG_Network, "SwitchToAdvancedSelectDelete");
    editable_state = EditableElement::ElementState::des_AdvancedSelectDelete;
}

void Network::SwitchToAdvancedSelectDisable() {
    ESP_LOGI(TAG_Network, "SwitchToAdvancedSelectDisable");
    editable_state = EditableElement::ElementState::des_AdvancedSelectDisable;
}

void Network::SwitchToMoving() {
    ESP_LOGI(TAG_Network, "SwitchToMoving");
    editable_state = EditableElement::ElementState::des_Moving;
}

void Network::SwitchToCopying() {
    ESP_LOGI(TAG_Network, "SwitchToCopying");
    editable_state = EditableElement::ElementState::des_Copying;
}

void Network::SwitchToDeleting() {
    ESP_LOGI(TAG_Network, "SwitchToDeleting");
    editable_state = EditableElement::ElementState::des_Deleting;
}

void Network::SwitchToDisabling() {
    ESP_LOGI(TAG_Network, "SwitchToDisabling");
    editable_state = EditableElement::ElementState::des_Disabling;
}

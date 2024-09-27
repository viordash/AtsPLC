#pragma once

#include "LogicProgram/LogicElement.h"
#include <stdint.h>
#include <unistd.h>
#include <vector>

class ElementsBox : public LogicElement, public std::vector<LogicElement *> {
  protected:
    uint8_t place_width;
    LogicElement *stored_element;
    int selected_index;

    void Fill();
    void AppendStandartElement(TvElementType element_type, uint8_t *frame_buffer);
    bool MatchedToStoredElement(TvElementType element_type);
    void TakeParamsFromStoredElement(LogicElement *new_element);

  public:
    ElementsBox(uint8_t place_width, LogicElement *stored_element);
    virtual ~ElementsBox();

    LogicElement *GetSelectedElement();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override final;
    bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override final;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;
    TvElementType GetElementType() override final;

    void HandleButtonUp();
    void HandleButtonDown();
    void HandleButtonSelect();
    void SelectedElementHandleButtonUp();
    void SelectedElementHandleButtonDown();
    void SelectedElementHandleButtonSelect();
};

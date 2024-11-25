#pragma once

#include "LogicProgram/Inputs/CommonInput.h"
#include "LogicProgram/Inputs/CommonTimer.h"
#include "LogicProgram/LogicElement.h"
#include "LogicProgram/Outputs/CommonOutput.h"
#include <stdint.h>
#include <unistd.h>
#include <vector>

class ElementsBox : public LogicElement, public std::vector<LogicElement *> {
  protected:
    uint8_t place_width;
    uint8_t source_element_width;    
    int selected_index;
    bool force_do_action_result;

    uint8_t CalcEntirePlaceWidth(uint8_t fill_wire, LogicElement *source_element);
    void Fill(LogicElement *source_element, bool hide_output_elements);
    void AppendStandartElement(LogicElement *source_element, TvElementType element_type, uint8_t *frame_buffer);
    bool CopyParamsToCommonInput(LogicElement *source_element, CommonInput *common_input);
    bool CopyParamsToCommonTimer(LogicElement *source_element, CommonTimer *common_timer);
    bool CopyParamsToCommonOutput(LogicElement *source_element, CommonOutput *common_output);
    void TakeParamsFromStoredElement(LogicElement *source_element, LogicElement *new_element);

  public:
    ElementsBox(uint8_t fill_wire, LogicElement *source_element, bool hide_output_elements);
    virtual ~ElementsBox();

    LogicElement *GetSelectedElement();

    bool DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) override final;
    bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override final;

    size_t Serialize(uint8_t *buffer, size_t buffer_size) override final;
    size_t Deserialize(uint8_t *buffer, size_t buffer_size) override final;
    TvElementType GetElementType() override final;

    void SelectPrior() override final;
    void SelectNext() override final;
    void PageUp() override final;
    void PageDown() override final;
    void Change() override final;
    bool EditingCompleted();
};
